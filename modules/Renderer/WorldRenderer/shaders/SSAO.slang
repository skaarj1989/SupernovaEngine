//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

#include "Resources/CameraBlock.h.slang"

struct PushConstants {
  float radius;
  float power;
  float bias;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

#ifndef KERNEL_SIZE
#  define KERNEL_SIZE 4
#endif

struct KernelBlock {
  float4 samples[KERNEL_SIZE];
};

[[vk::binding(1, 2)]]
ConstantBuffer<KernelBlock> u_Kernel;

static const float kInvKernelSize = 1.0 / KERNEL_SIZE;

[[vk::binding(2, 2)]]
Sampler2D t_Noise;

[[vk::binding(0, 2)]]
SamplerState s_0;
[[vk::binding(5, 1)]]
Texture2D t_SceneDepth;
[[vk::binding(6, 1)]]
Texture2D t_GBuffer0; // .rgb = Normal

#include "Lib/Depth.h.slang"

[shader("fragment")]
float main(VSOutput input) : SV_Target {
  const float depth = getDepth(t_SceneDepth, s_0, input.texCoord);
  if (depth >= 1.0) discard;

  float2 gBufferSize;
  t_SceneDepth.GetDimensions(gBufferSize.x, gBufferSize.y);
  float2 noiseSize;
  t_Noise.GetDimensions(noiseSize.x, noiseSize.y);
  const float2 noiseTexCoord = (gBufferSize / noiseSize) * input.texCoord;
  const float3 rvec = float3(t_Noise.Sample(noiseTexCoord).xy, 0.0);

  float3 N = t_GBuffer0.Sample(s_0, input.texCoord).rgb;
  N = mul(float3x3(u_Camera.view), normalize(N));
  const float3 T = normalize(rvec - N * dot(rvec, N));
  const float3 B = cross(N, T);
  const float3x3 TBN = float3x3(T, B, N); // tangent-space -> view-space

  const float3 fragPosViewSpace = viewPositionFromDepth(depth, input.texCoord);

  float occlusion = 0.0;
  for (uint i = 0; i < KERNEL_SIZE; ++i) {
    float3 samplePos = mul(TBN, u_Kernel.samples[i].xyz);
    samplePos = fragPosViewSpace + samplePos * u_PC.radius;
    const float2 offset = viewToNDC(float4(samplePos, 1.0)).xy * 0.5 + 0.5;
    float sampleDepth = getDepth(t_SceneDepth, s_0, offset);
    sampleDepth = viewPositionFromDepth(sampleDepth, offset).z;

    const float rangeCheck =
      smoothstep(0.0, 1.0, u_PC.radius / abs(fragPosViewSpace.z - sampleDepth));
    occlusion +=
      (sampleDepth >= samplePos.z + u_PC.bias ? 1.0 : 0.0) * rangeCheck;
  }

  // 1.0 = no occlusion, 0.0 = occluded
  occlusion = 1.0 - occlusion * kInvKernelSize;
  return pow(occlusion, u_PC.power);
}
