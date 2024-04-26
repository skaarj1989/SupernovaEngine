#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

// Use with Mesh.vert

#include "VertexData.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"
#include "Resources/IDs.glsl"

#include "Lib/Depth.glsl"

layout(set = 0, binding = 4) uniform samplerShadow s_Shadow;
layout(set = 0, binding = 5) uniform samplerShadow s_OmniShadow;

#if HAS_SCENE_DEPTH
layout(set = 1, binding = 5) uniform sampler2D t_SceneDepth;
#endif
#if HAS_SCENE_COLOR
layout(set = 1, binding = 11) uniform sampler2D t_SceneColor;
#endif

#include "Lib/Light.glsl"
_DECLARE_LIGHT_BUFFER(1, 3, g_LightBuffer);

layout(set = 2, binding = 0) uniform sampler2D t_BRDF;
#if HAS_SKYLIGHT
layout(set = 2, binding = 1) uniform samplerCube t_IrradianceMap;
layout(set = 2, binding = 2) uniform samplerCube t_PrefilteredEnvMap;
#endif

#if TILED_LIGHTING
layout(set = 2, binding = 8, std430) buffer readonly _LightIndexList {
  // .x = indices for opaque geometry
  // .y = indices for transparent geometry
  uvec2 g_LightIndexList[];
};

// [startOffset, lightCount]
// .rg = opaque geometry
// .ba = transparent geometry
layout(set = 2, binding = 7, rgba32ui) uniform readonly uimage2D i_LightGrid;
#endif

layout(set = 2, binding = 3) uniform texture2DArray t_CascadedShadowMaps;
layout(set = 2, binding = 4) uniform texture2DArray t_SpotLightShadowMaps;
layout(set = 2, binding = 5) uniform textureCubeArray t_OmniShadowMaps;

#include "Resources/ShadowBlock.glsl"
#include "Lib/CSM.glsl"
#include "Lib/SpotLightShadow.glsl"
#include "Lib/OmniShadow.glsl"

#include "Material.glsl"

layout(push_constant) uniform _PushConstants {
  layout(offset = 16) vec4 ambientLight;
  float IBLIntensity;
  float GIIntensity;
}
u_PC;

#include "Lib/IBL.glsl"

#if WEIGHTED_BLENDED
layout(location = 0) out vec4 Accum;
layout(location = 1) out float Reveal;
#else
layout(location = 0) out vec4 FragColor;
#endif

void main() {
  const FragmentPosition fragPos =
    buildFragPosFromWorldSpace(fs_in.fragPosWorldSpace);

  Material material = _initMaterial();
  // clang-format off
  _executeUserCode(
    fragPos,
#if HAS_PROPERTIES
    u_Properties[fs_in.materialId],
#endif
    material
  );
  // clang-format on

#if SHADING_MODEL == SHADING_MODEL_UNLIT
  const vec3 color = material.emissiveColor;
#else
  material.metallic = clamp01(material.metallic);
  material.roughness = clamp01(material.roughness);

  const vec3 N = normalize(material.normal);
  const vec3 V = getViewDir();
  const float NdotV = clamp01(dot(N, V));

  const bool receiveShadow =
    (fs_in.flags & MaterialFlag_ReceiveShadow) == MaterialFlag_ReceiveShadow;

  // Metallic-Roughness workflow

  // Dielectrics: [0.02..0.05], Metals: [0.5..1.0]
  const float kMinRoughness = 0.04;
  vec3 F0 = vec3(kMinRoughness);
  const vec3 diffuseColor =
    mix(material.baseColor.rgb, vec3(0.0), material.metallic);
  F0 = mix(F0, material.baseColor, material.metallic);
#  if IS_TRANSMISSIVE
  F0 = vec3(pow((material.ior - 1.0) / (material.ior + 1.0), 2.0));
#  endif
  const vec3 F90 = vec3(1.0);

  const float alphaRoughness = material.roughness * material.roughness;

  //
  // Ambient lighting:
  //

  vec3 Lo_diffuse = vec3(0.0);
  vec3 Lo_specular = vec3(0.0);
  vec3 Lo_transmission = vec3(0.0);

  Lo_diffuse +=
    material.baseColor * u_PC.ambientLight.rgb * u_PC.ambientLight.a;

#  if HAS_SKYLIGHT
  // clang-format off
  const LightContribution ambientLighting = IBL_AmbientLighting(
    diffuseColor,
    F0,
    material.specular,
    material.roughness,
    N,
    V,
    NdotV
  );
  // clang-format on
  Lo_diffuse += ambientLighting.diffuse;
  Lo_specular += ambientLighting.specular;
#  endif

#  if IS_TRANSMISSIVE
  // clang-format off
  Lo_transmission += getIBLVolumeRefraction(
    N,
    V,
    material.roughness,
    diffuseColor,
    F0,
    F90,
    fragPos.worldSpace,
    material.ior,
    material.thickness,
    material.attenuationColor,
    material.attenuationDistance
  );
  // clang-format on
#  endif

  //
  // Direct lighting:
  //

#  if TILED_LIGHTING
  const ivec2 tileId = ivec2(floor(gl_FragCoord.xy / float(TILE_SIZE)));
  const uvec2 tileInfo = imageLoad(i_LightGrid, tileId).ba;
  const uint startOffset = tileInfo.x;
  const uint lightCount = tileInfo.y;

  for (uint i = 0; i < lightCount; ++i) {
    const uint lightIndex = g_LightIndexList[startOffset + i].y;
#  else
  for (uint i = 0; i < g_LightBuffer.numLights; ++i) {
    const uint lightIndex = i;
#  endif
    const Light light = g_LightBuffer.data[lightIndex];

    vec3 fragToLight = _getFragToLight(light, fragPos.worldSpace);
    vec3 L = normalize(fragToLight);
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
        BRDF_lambertian(F0, F90, diffuseColor, material.specular, VdotH);
      Lo_specular +=
        radiance * NdotL *
        BRDF_specularGGX(F0, F90, alphaRoughness, material.specular, VdotH,
                         NdotL, NdotV, NdotH);
    }

#  if IS_TRANSMISSIVE
    // BTDF
    const vec3 transmissionRay = getVolumeTransmissionRay(
      N, V, material.thickness, material.ior, fs_in.modelMatrix);
    fragToLight -= transmissionRay;
    L = normalize(fragToLight);

    const vec3 intensity = _getLightIntensity(light, fragToLight);

    vec3 transmittedLight = intensity * getPunctualRadianceTransmission(
                                          N, V, L, alphaRoughness, F0, F90,
                                          diffuseColor, material.ior);

    // VOLUME
    transmittedLight = applyVolumeAttenuation(
      transmittedLight, length(transmissionRay), material.attenuationColor,
      material.attenuationDistance);

    Lo_transmission += transmittedLight;
#  endif
  }

  // ---

#  if IS_TRANSMISSIVE
  Lo_diffuse =
    mix(Lo_diffuse, Lo_transmission, clamp01(material.transmissionFactor));
#  endif

  const vec3 color = Lo_diffuse + Lo_specular + material.emissiveColor;
#endif

  const float a = clamp01(material.opacity);
#if WEIGHTED_BLENDED
  const float w = clamp(pow(min(1.0, a * 10.0) + 0.01, 3.0) * 1e8 *
                          pow(1.0 - gl_FragCoord.z * 0.9, 3.0),
                        1e-2, 3e3);

  Accum = vec4(color * a, a) * w;
  Reveal = a;
#else
  FragColor = vec4(color, a);
#endif
#if WRITE_USERDATA
  writeUserData(fs_in.userData);
#endif
}
