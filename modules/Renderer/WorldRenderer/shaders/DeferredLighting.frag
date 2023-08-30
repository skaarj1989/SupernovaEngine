#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"

#include "Lib/Depth.glsl"
#include "FragmentPosition.glsl"

layout(set = 0, binding = 3) uniform sampler s_Point;
layout(set = 0, binding = 4) uniform sampler s_Bilinear;
layout(set = 0, binding = 5) uniform samplerShadow s_Shadow;
layout(set = 0, binding = 6) uniform samplerShadow s_OmniShadow;

layout(set = 1, binding = 5) uniform texture2D t_GBufferDepth;
layout(set = 1, binding = 6) uniform texture2D t_GBufferNormal;
layout(set = 1, binding = 7) uniform texture2D t_GBufferEmissive;
layout(set = 1, binding = 8) uniform texture2D t_GBufferAlbedoSpecular;
layout(set = 1, binding = 9) uniform texture2D t_GBufferMetallicRoughnessAO;
layout(set = 1, binding = 10) uniform texture2D t_GBufferMisc;

#include "Lib/Light.glsl"
_DECLARE_LIGHT_BUFFER(1, 3, g_LightBuffer);

#if HAS_SKYLIGHT
layout(set = 2, binding = 0) uniform sampler2D t_BRDF;
layout(set = 2, binding = 1) uniform samplerCube t_IrradianceMap;
layout(set = 2, binding = 2) uniform samplerCube t_PrefilteredEnvMap;
#endif

#if TILED_LIGHTING
layout(set = 2, binding = 8, std430) buffer readonly LightIndexList {
  // .x = indices for opaque geometry
  // .y = indices for transparent geometry
  uvec2 g_LightIndexList[];
};

// [startOffset, lightCount]
// .rg = opaque geometry
// .ba = transparent geometry
layout(set = 2, binding = 7, rgba32ui) uniform readonly uimage2D i_LightGrid;
#endif

#if HAS_SSAO
layout(set = 2, binding = 6) uniform texture2D t_SSAO;
#endif

#include "Resources/ShadowBlock.glsl"

layout(set = 2, binding = 3) uniform texture2DArray t_CascadedShadowMaps;
#include "Lib/CSM.glsl"

layout(set = 2, binding = 4) uniform texture2DArray t_SpotLightShadowMaps;
#include "Lib/SpotLightShadow.glsl"

layout(set = 2, binding = 5) uniform textureCubeArray t_OmniShadowMaps;
#include "Lib/OmniShadow.glsl"

#if HAS_GLOBAL_ILLUMINATION
#  include "Resources/SceneGridBlock.glsl"
layout(set = 2, binding = 10) uniform texture3D t_AccumulatedSH_R;
layout(set = 2, binding = 11) uniform texture3D t_AccumulatedSH_G;
layout(set = 2, binding = 12) uniform texture3D t_AccumulatedSH_B;

#  include "Lib/LPV.glsl"

bool inBounds(vec3 v) {
  return all(greaterThanEqual(v, vec3(0.0))) &&
         all(lessThanEqual(v, vec3(1.0)));
}
#endif

layout(push_constant) uniform _PushConstants {
  vec4 ambientLight;
  float IBLIntensity;
  float GIIntensity;
}
u_PC;

#include "Lib/IBL.glsl"

#include "MaterialDefines.glsl"
#include "Lib/GBufferHelper.glsl"

layout(location = 0) out vec3 FragColor;
void main() {
  const FragmentPosition fragPos =
    buildFragPosFromDepthSampler(t_GBufferDepth, s_Point, v_TexCoord);

  const vec3 emissiveColor =
    texture(sampler2D(t_GBufferEmissive, s_Point), v_TexCoord).rgb;

  const uint encoded =
    sampleEncodedMiscData(t_GBufferMisc, s_Point, v_TexCoord);
  const uint shadingModel = decodeShadingModel(encoded);
  if (shadingModel == SHADING_MODEL_UNLIT) {
    FragColor = emissiveColor;
    return;
  }
  const uint materialFlags = decodeMaterialFlags(encoded);
  const bool receiveShadow =
    (materialFlags & MaterialFlag_ReceiveShadow) == MaterialFlag_ReceiveShadow;

  vec4 temp =
    texture(sampler2D(t_GBufferMetallicRoughnessAO, s_Point), v_TexCoord);
  // metallic and rougnes already clamped [0..1] (see GBufferPass)
  const float metallic = temp.r;
  const float roughness = temp.g;
  float ao = temp.b;
#if HAS_SSAO
  ao = min(ao, texture(sampler2D(t_SSAO, s_Point), v_TexCoord).r);
#endif

  temp = texture(sampler2D(t_GBufferAlbedoSpecular, s_Point), v_TexCoord);
  const vec3 albedo = temp.rgb;
  const float specularWeight = temp.a;

  //
  // Lighting calculation:
  //

  // NOTE: values in world-space
  // N = normal (from surface)
  // L = fragment to light direction
  // V = fragment to eye direction
  // H = halfway vector (between V and L)

  const vec3 N =
    normalize(texture(sampler2D(t_GBufferNormal, s_Point), v_TexCoord).rgb);
  const vec3 V = normalize(getCameraPosition() - fragPos.worldSpace);
  const float NdotV = clamp01(dot(N, V));

  // Metallic-Roughness workflow

  // Dielectrics: [0.02..0.05], Metals: [0.5..1.0]
  const float kMinRoughness = 0.04;
  vec3 F0 = vec3(kMinRoughness);
  // Achromatic F0 based on IOR.
  const vec3 diffuseColor = mix(albedo, vec3(0.0), metallic);
  F0 = mix(F0, albedo, metallic);
  const vec3 F90 = vec3(1.0);

  const float alphaRoughness = roughness * roughness;

  //
  // Ambient lighting:
  //

  vec3 Lo_diffuse = vec3(0.0);
  vec3 Lo_specular = vec3(0.0);

  Lo_diffuse += albedo * u_PC.ambientLight.rgb * u_PC.ambientLight.a * ao;

#if HAS_SKYLIGHT
  // clang-format off
  const LightContribution ambientLighting = IBL_AmbientLighting(
    diffuseColor,
    F0,
    specularWeight,
    roughness,
    N,
    V,
    NdotV
  );
  // clang-format on
  Lo_diffuse += ambientLighting.diffuse * ao;
  Lo_specular += ambientLighting.specular * ao;
#endif

#if HAS_GLOBAL_ILLUMINATION
  const vec3 cellCoords = (fragPos.worldSpace - u_SceneGrid.minCorner) /
                          u_SceneGrid.cellSize / vec3(u_SceneGrid.gridSize);
  if (inBounds(cellCoords)) {
    // clang-format off
    const SHcoeffs coeffs = {
      texture(sampler3D(t_AccumulatedSH_R, s_Bilinear), cellCoords, 0),
      texture(sampler3D(t_AccumulatedSH_G, s_Bilinear), cellCoords, 0),
      texture(sampler3D(t_AccumulatedSH_B, s_Bilinear), cellCoords, 0)
    };
    const vec4 SH_intensity = SH_evaluate(-N);
    const vec3 LPV_intensity =
      vec3(
        dot(SH_intensity, coeffs.red),
        dot(SH_intensity, coeffs.green),
        dot(SH_intensity, coeffs.blue)
      );
    // clang-format on
    const vec3 LPV_radiance = max(
      LPV_intensity * 4.0 / u_SceneGrid.cellSize / u_SceneGrid.cellSize, 0.0);

#  if IRRADIANCE_ONLY
    FragColor = LPV_radiance;
    return;
#  else
    Lo_diffuse += u_PC.GIIntensity * albedo * LPV_radiance * ao;
#  endif
  }
#endif

  //
  // Direct lighting:
  //

#if TILED_LIGHTING
  const ivec2 tileId = ivec2(floor(gl_FragCoord.xy / float(TILE_SIZE)));
  const uvec2 tileInfo = imageLoad(i_LightGrid, tileId).rg;
  const uint startOffset = tileInfo.x;
  const uint lightCount = tileInfo.y;

  for (uint i = 0; i < lightCount; ++i) {
    const uint lightIndex = g_LightIndexList[startOffset + i].x;
#else
  for (uint i = 0; i < g_LightBuffer.numLights; ++i) {
    const uint lightIndex = i;
#endif
    const Light light = g_LightBuffer.data[lightIndex];

    const vec3 fragToLight = _getFragToLight(light, fragPos.worldSpace);
    const vec3 L = normalize(fragToLight);
    const vec3 H = normalize(V + L);

    const float NdotL = clamp01(dot(N, L));
    if (NdotL > 0.0 || NdotV > 0.0) {
      float visibility = 1.0;
      if (receiveShadow && light.shadowMapId != -1) {
        switch (light.type) {
        case LightType_Directional:
          const uint cascadeIndex = _selectCascadeIndex(fragPos.viewSpace);
          visibility =
            _getDirLightVisibility(cascadeIndex, t_CascadedShadowMaps, s_Shadow,
                                   light, fragPos.worldSpace, NdotL);
          break;

        case LightType_Spot:
          visibility = _getSpotLightVisibility(
            t_SpotLightShadowMaps, s_Shadow, light, fragPos.worldSpace, NdotL);
          break;

        case LightType_Point:
          visibility = _getPointLightVisibility(
            t_OmniShadowMaps, s_OmniShadow, light, fragPos.worldSpace, NdotL);
          break;
        }

        if (visibility == 0.0) continue;
      }

      const vec3 radiance =
        _getLightIntensity(light, fragToLight) * NdotL * visibility;

      const float VdotH = clamp01(dot(V, H));
      const float NdotH = clamp01(dot(N, H));

      Lo_diffuse +=
        radiance * NdotL *
        BRDF_lambertian(F0, F90, diffuseColor, specularWeight, VdotH);
      Lo_specular += radiance * NdotL *
                     BRDF_specularGGX(F0, F90, alphaRoughness, specularWeight,
                                      VdotH, NdotL, NdotV, NdotH);
    }
  }

  FragColor = Lo_diffuse + Lo_specular + emissiveColor;
}
