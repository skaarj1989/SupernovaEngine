//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

// http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html

#include "Resources/CameraBlock.h.slang"

[[vk::binding(3, 0)]]
SamplerState s_0;

[[vk::binding(5, 1)]]
Texture2D t_SceneDepth;

[[vk::binding(6, 1)]]
Texture2D t_GBuffer0; // .rgb = Normal

[[vk::binding(9, 1)]]
Texture2D t_GBuffer3; // .r = Metallic, .g = Roughness, .b = AO, .a = UNUSED

[[vk::binding(10, 1)]]
Texture2D t_GBuffer4; // .r = ShadingModel/MaterialFlags

[[vk::binding(11, 1)]]
Texture2D t_SceneColor;

#include "Lib/Depth.h.slang"
#include "Lib/BRDF.h.slang"
#include "Lib/RayCast.h.slang"

#include "MaterialDefines.h.slang"
#include "Lib/GBufferHelper.h.slang"

float3 hash(float3 a) {
  const float kScale = 0.8;
  const float K = 19.19;

  a = fract(a * kScale);
  a += dot(a, a.yxz + K);
  return fract((a.xxy + a.yxx) * a.zyx);
}

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  const float depth = getDepth(t_SceneDepth, s_0, input.texCoord);
  if (depth >= 1.0) discard;

  const uint encoded = sampleEncodedMiscData(t_GBuffer4, s_0, input.texCoord);
  const uint shadingModel = decodeShadingModel(encoded);
  if (shadingModel == SHADING_MODEL_UNLIT) discard;

  const float4 temp = t_GBuffer3.Sample(s_0, input.texCoord);
  const float metallic = clamp01(temp.r);
  const float roughness = clamp01(temp.g);
  if (roughness > 0.9 || metallic < 0.1) discard;

  float3 N = normalize(t_GBuffer0.Sample(s_0, input.texCoord).xyz);
  N = mul(float3x3(u_Camera.view), N);

  const float3 fragPosViewSpace = viewPositionFromDepth(depth, input.texCoord);
  const float3 V = normalize(-fragPosViewSpace);

  const float NdotV = clamp01(dot(N, V));

  float3 jitter = hash(fragPosViewSpace) * roughness * 0.1;
  jitter.x = 0.0;

  const float3 R = normalize(reflect(-V, N));
  const RayCastResult rayResult =
    rayCast(fragPosViewSpace, normalize(R + jitter), 0.1, 500, 10);
  if (!rayResult.hit) discard;

  const float3 reflectedColor = t_SceneColor.Sample(s_0, rayResult.uv).rgb;

  const float3 albedo = t_SceneColor.Sample(s_0, input.texCoord).rgb;
  const float kMinRoughness = 0.04;
  float3 F0 = float3(kMinRoughness);
  F0 = lerp(F0, albedo, metallic);
  const float3 F90 = float3(1.0);
  const float3 F = F_Schlick(F0, F90, NdotV);
  const float visibility = 1.0 - max(dot(V, R), 0.0);

  const float2 dCoords = smoothstep(0.2, 0.6, abs(0.5 - rayResult.uv));
  const float screenEdgeFactor = clamp01(1.0 - (dCoords.x + dCoords.y));
  const float kReflectionSpecularFalloffExponent = 3.0;
  const float reflectionMultiplier =
    pow(metallic, kReflectionSpecularFalloffExponent) * screenEdgeFactor * -R.z;

  return float4(reflectedColor * F * visibility * clamp01(reflectionMultiplier),
                visibility);
}
