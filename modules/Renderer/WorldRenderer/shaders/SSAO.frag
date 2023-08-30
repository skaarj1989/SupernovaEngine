#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"

layout(set = 2, binding = 1, std140) uniform _KernelBlock {
  vec4 samples[KERNEL_SIZE];
}
u_Kernel;

const uint kNumSamples = u_Kernel.samples.length();
const float kInvKernelSize = 1.0 / kNumSamples;

layout(set = 2, binding = 2) uniform sampler2D t_Noise;

layout(set = 2, binding = 0) uniform sampler s_0;
layout(set = 1, binding = 5) uniform texture2D t_SceneDepth;
layout(set = 1, binding = 6) uniform texture2D t_GBuffer0; // .rgb = Normal

layout(push_constant) uniform _PushConstants {
  float radius;
  float power;
  float bias;
}
u_PC;

#include "Lib/Depth.glsl"

layout(location = 0) out float FragColor;
void main() {
  const float depth = getDepth(sampler2D(t_SceneDepth, s_0), v_TexCoord);
  if (depth >= 1.0) discard;

  const vec2 gBufferSize = textureSize(t_SceneDepth, 0);
  const vec2 noiseSize = textureSize(t_Noise, 0);
  const vec2 noiseTexCoord = (gBufferSize / noiseSize) * v_TexCoord;
  const vec3 rvec = vec3(texture(t_Noise, noiseTexCoord).xy, 0.0);

  vec3 N = texture(sampler2D(t_GBuffer0, s_0), v_TexCoord).rgb;
  N = mat3(u_Camera.view) * normalize(N);
  const vec3 T = normalize(rvec - N * dot(rvec, N));
  const vec3 B = cross(N, T);
  const mat3 TBN = mat3(T, B, N); // tangent-space -> view-space

  const vec3 fragPosViewSpace = viewPositionFromDepth(depth, v_TexCoord);

  float occlusion = 0.0;
  for (uint i = 0; i < kNumSamples; ++i) {
    vec3 samplePos = TBN * u_Kernel.samples[i].xyz;
    samplePos = fragPosViewSpace + samplePos * u_PC.radius;
    const vec2 offset = viewToNDC(vec4(samplePos, 1.0)).xy * 0.5 + 0.5;
    float sampleDepth = getDepth(sampler2D(t_SceneDepth, s_0), offset);
    sampleDepth = viewPositionFromDepth(sampleDepth, offset).z;

    const float rangeCheck =
      smoothstep(0.0, 1.0, u_PC.radius / abs(fragPosViewSpace.z - sampleDepth));
    occlusion +=
      (sampleDepth >= samplePos.z + u_PC.bias ? 1.0 : 0.0) * rangeCheck;
  }

  // 1.0 = no occlusion, 0.0 = occluded
  occlusion = 1.0 - occlusion * kInvKernelSize;
  FragColor = pow(occlusion, u_PC.power);
}
