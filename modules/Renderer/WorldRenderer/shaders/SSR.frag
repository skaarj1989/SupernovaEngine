#version 460 core

// http://imanolfotia.com/blog/update/2017/03/11/ScreenSpaceReflections.html

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"

layout(set = 0, binding = 3) uniform sampler s_0;
layout(set = 1, binding = 5) uniform texture2D t_SceneDepth;

layout(set = 1, binding = 6) uniform texture2D t_GBuffer0; // .rgb = Normal
// .r = Metallic, .g = Roughness, .b = AO, .a = UNUSED
layout(set = 1, binding = 9) uniform texture2D t_GBuffer3;
// .r = ShadingModel/MaterialFlags
layout(set = 1, binding = 10) uniform texture2D t_GBuffer4;

layout(set = 1, binding = 11) uniform texture2D t_SceneColor;

#include "Lib/Depth.glsl"
#include "Lib/BRDF.glsl"
#include "Lib/RayCast.glsl"

#include "MaterialDefines.glsl"
#include "Lib/GBufferHelper.glsl"

vec3 hash(vec3 a) {
  const float kScale = 0.8;
  const float K = 19.19;

  a = fract(a * kScale);
  a += dot(a, a.yxz + K);
  return fract((a.xxy + a.yxx) * a.zyx);
}

layout(location = 0) out vec4 FragColor;
void main() {
  const float depth = getDepth(sampler2D(t_SceneDepth, s_0), v_TexCoord);
  if (depth >= 1.0) discard;

  const uint encoded = sampleEncodedMiscData(t_GBuffer4, s_0, v_TexCoord);
  const uint shadingModel = decodeShadingModel(encoded);
  if (shadingModel == SHADING_MODEL_UNLIT) discard;

  const vec4 temp = texture(sampler2D(t_GBuffer3, s_0), v_TexCoord);
  const float metallic = clamp01(temp.r);
  const float roughness = clamp01(temp.g);
  if (roughness > 0.9 || metallic < 0.1) discard;

  vec3 N = normalize(texture(sampler2D(t_GBuffer0, s_0), v_TexCoord).xyz);
  N = mat3(u_Camera.view) * N;

  const vec3 fragPosViewSpace = viewPositionFromDepth(depth, v_TexCoord);
  const vec3 V = normalize(-fragPosViewSpace);

  const float NdotV = clamp01(dot(N, V));

  vec3 jitter = hash(fragPosViewSpace) * roughness * 0.1;
  jitter.x = 0.0;

  const vec3 R = normalize(reflect(-V, N));
  const RayCastResult rayResult =
    rayCast(fragPosViewSpace, normalize(R + jitter), 0.1, 500, 10);
  if (!rayResult.hit) discard;

  const vec3 reflectedColor =
    texture(sampler2D(t_SceneColor, s_0), rayResult.uv).rgb;

  const vec3 albedo = texture(sampler2D(t_SceneColor, s_0), v_TexCoord).rgb;
  const float kMinRoughness = 0.04;
  vec3 F0 = vec3(kMinRoughness);
  F0 = mix(F0, albedo, metallic);
  const vec3 F90 = vec3(1.0);
  const vec3 F = F_Schlick(F0, F90, NdotV);
  const float visibility = 1.0 - max(dot(V, R), 0.0);

  const vec2 dCoords = smoothstep(0.2, 0.6, abs(0.5 - rayResult.uv));
  const float screenEdgeFactor = clamp01(1.0 - (dCoords.x + dCoords.y));
  const float kReflectionSpecularFalloffExponent = 3.0;
  const float reflectionMultiplier =
    pow(metallic, kReflectionSpecularFalloffExponent) * screenEdgeFactor * -R.z;

  FragColor =
    vec4(reflectedColor * F * visibility * clamp01(reflectionMultiplier),
         visibility);
}
