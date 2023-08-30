#ifndef _IBL_GLSL_
#define _IBL_GLSL_

#include "BRDF.glsl"
#include "Texture.glsl"
#include "DecomposeMatrix.glsl"

#if IS_TRANSMISSIVE

vec3 getVolumeTransmissionRay(vec3 N, vec3 V, float thickness, float ior,
                              mat4 modelMatrix) {
  // Direction of refracted light.
  vec3 refractionVector = refract(-V, N, 1.0 / ior);
  // The thickness is specified in local space.
  return refractionVector * thickness * decomposeScale(modelMatrix);
}

// Compute attenuated light as it travels through a volume.
vec3 applyVolumeAttenuation(vec3 radiance, float transmissionDistance,
                            vec3 attenuationColor, float attenuationDistance) {
  if (attenuationDistance == 0.0) {
    // Attenuation distance is +∞ (which we indicate by zero), i.e. the
    // transmitted color is not attenuated at all.
    return radiance;
  } else {
    // Compute light attenuation using Beer's law.
    const vec3 attenuationCoefficient =
      -log(attenuationColor) / attenuationDistance;
    const vec3 transmittance =
      exp(-attenuationCoefficient * transmissionDistance); // Beer's law
    return transmittance * radiance;
  }
}

float applyIorToRoughness(float roughness, float ior) {
  // Scale roughness with IOR so that an IOR of 1.0 results in no microfacet
  // refraction and an IOR of 1.5 results in the default amount of microfacet
  // refraction.
  return roughness * clamp01(ior * 2.0 - 2.0);
}

vec3 getPunctualRadianceTransmission(vec3 N, vec3 V, vec3 fragToLight,
                                     float alphaRoughness, vec3 F0, vec3 F90,
                                     vec3 baseColor, float ior) {
  const float transmissionRougness = applyIorToRoughness(alphaRoughness, ior);

  const vec3 L = normalize(fragToLight);
  const vec3 L_mirror = normalize(
    L + 2.0 * N * dot(-L, N)); // Mirror light reflection vector on surface
  const vec3 H = normalize(
    L_mirror + V); // Halfway vector between transmission light vector and v

  const float D = D_GGX(clamp01(dot(N, H)), transmissionRougness);
  const vec3 F = F_Schlick(F0, F90, clamp01(dot(V, H)));
  const float Vis =
    V_GGX(clamp01(dot(N, L_mirror)), clamp01(dot(N, V)), transmissionRougness);

  // Transmission BTDF
  return (1.0 - F) * baseColor * D * Vis;
}

vec3 getTransmissionSample(vec2 fragCoord, float roughness, float ior) {
#  if 0
    // No LODs in the SceneColor
    float framebufferLod = log2(float(textureSize(t_SceneColor, 0).x)) *
                           applyIorToRoughness(roughness, ior);
#  endif
  const float framebufferLod = 0.0;
  const vec3 transmittedLight =
    textureLod(t_SceneColor, fragCoord.xy, framebufferLod).rgb;
  return transmittedLight;
}

vec3 getIBLVolumeRefraction(vec3 N, vec3 V, float perceptualRoughness,
                            vec3 baseColor, vec3 F0, vec3 F90, vec3 position,
                            float ior, float thickness, vec3 attenuationColor,
                            float attenuationDistance) {
  const vec3 transmissionRay =
    getVolumeTransmissionRay(N, V, thickness, ior, fs_in.modelMatrix);
  const vec3 refractedRayExit = position + transmissionRay;

  // Project refracted vector on the framebuffer, while mapping to normalized
  // device coordinates.
  const vec3 ndc = worldToNDC(vec4(refractedRayExit, 1.0));
  const vec2 refractionCoords = ndc.xy * 0.5 + 0.5;

  // Sample framebuffer to get pixel the refracted ray hits.
  const vec3 transmittedLight =
    getTransmissionSample(refractionCoords, perceptualRoughness, ior);

  const vec3 attenuatedColor =
    applyVolumeAttenuation(transmittedLight, length(transmissionRay),
                           attenuationColor, attenuationDistance);

  // Sample GGX LUT to get the specular component.
  const float NdotV = clamp01(dot(N, V));
  const vec2 brdfSamplePoint = vec2(NdotV, perceptualRoughness);
  const vec2 brdf = texture(t_BRDF, brdfSamplePoint).rg;
  const vec3 specularColor = F0 * brdf.x + F90 * brdf.y;

  return (1.0 - specularColor) * attenuatedColor * baseColor;
}

#endif

#if HAS_SKYLIGHT

vec3 getDiffuseLight(vec3 N) {
  return texture(t_IrradianceMap, N).rgb * u_PC.IBLIntensity;
}

vec3 getSpecularLight(vec3 R, float roughness) {
  const float mipCount = calculateMipLevels(t_PrefilteredEnvMap);
  const float lod = roughness * float(mipCount - 1);
  return textureLod(t_PrefilteredEnvMap, R, lod).rgb * u_PC.IBLIntensity;
}

LightContribution IBL_AmbientLighting(vec3 diffuseColor, vec3 F0,
                                      float specularWeight, float roughness,
                                      vec3 N, vec3 V, float NdotV) {
  const vec2 f_ab = textureLod(t_BRDF, clamp01(vec2(NdotV, roughness)), 0.0).rg;
  const vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
  const vec3 k_S = F0 + Fr * pow(1.0 - NdotV, 5.0);

  // -- Diffuse IBL:

  vec3 FssEss = specularWeight * k_S * f_ab.x + f_ab.y;

  // Multiple scattering, from Fdez-Aguera
  const float Ems = (1.0 - (f_ab.x + f_ab.y));
  const vec3 F_avg = specularWeight * (F0 + (1.0 - F0) / 21.0);
  const vec3 FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);
  const vec3 k_D = diffuseColor * (1.0 - FssEss + FmsEms);

  const vec3 irradiance = getDiffuseLight(N);
  const vec3 diffuse = (FmsEms + k_D) * irradiance;

  // -- Specular IBL:

  const vec3 R = normalize(reflect(-V, N));
  const vec3 specularLight = getSpecularLight(R, roughness);
  FssEss = k_S * f_ab.x + f_ab.y;
  const vec3 specular = specularWeight * specularLight * FssEss;

  // ---

  return LightContribution(diffuse, specular);
}

#endif

#endif
