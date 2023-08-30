#ifndef _BRDF_GLSL_
#define _BRDF_GLSL_

// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/ba6678f35ce33d7f48c00dd070435207efafaa46/source/Renderer/shaders/brdf.glsl

#include "Math.glsl"

// NOTE: a = alphaRoughness = (roughness * roughness)

vec3 F_Schlick(vec3 F0, vec3 F90, float VdotH) {
  return F0 + (F90 - F0) * pow(clamp01(1.0 - VdotH), 5.0);
}
float F_Schlick(float F0, float F90, float VdotH) {
  const float x = clamp01(1.0 - VdotH);
  const float x2 = x * x;
  const float x5 = x * x2 * x2;
  return F0 + (F90 - F0) * x5;
}
float F_Schlick(float F0, float VdotH) {
  float F90 = 1.0; // clamp01(50.0 * F0);
  return F_Schlick(F0, F90, VdotH);
}
vec3 F_Schlick(vec3 F0, float F90, float VdotH) {
  const float x = clamp01(1.0 - VdotH);
  const float x2 = x * x;
  const float x5 = x * x2 * x2;
  return F0 + (F90 - F0) * x5;
}
vec3 F_Schlick(vec3 F0, float VdotH) {
  float F90 = 1.0; // clamp01(dot(F0, vec3(50.0 * 0.33)));
  return F_Schlick(F0, F90, VdotH);
}

// Smith Joint GGX
// Note: Vis = G / (4 * NdotL * NdotV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in
// Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3 see
// Real-Time Rendering. Page 331 to 336. see
// https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float V_GGX(float NdotL, float NdotV, float alphaRoughness) {
  const float alphaRoughnessSq = alphaRoughness * alphaRoughness;

  const float GGXV =
    NdotL * sqrt(NdotV * NdotV * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);
  const float GGXL =
    NdotV * sqrt(NdotL * NdotL * (1.0 - alphaRoughnessSq) + alphaRoughnessSq);

  const float GGX = GGXV + GGXL;
  return GGX > 0.0 ? 0.5 / GGX : 0.0;
}

// The following equation(s) model the distribution of microfacet normals across
// the area being drawn (aka D()) Implementation from "Average Irregularity
// Representation of a Roughened Surface for Ray Reflection" by T. S.
// Trowbridge, and K. P. Reitz Follows the distribution function recommended in
// the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float D_GGX(float NdotH, float alphaRoughness) {
  const float alphaRoughnessSq = alphaRoughness * alphaRoughness;
  const float f = (NdotH * NdotH) * (alphaRoughnessSq - 1.0) + 1.0;
  return alphaRoughnessSq / (PI * f * f);
}

// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments
// AppendixB
vec3 BRDF_lambertian(vec3 F0, vec3 F90, vec3 diffuseColor, float specularWeight,
                     float VdotH) {
  // see
  // https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
  return (1.0 - specularWeight * F_Schlick(F0, F90, VdotH)) *
         (diffuseColor / PI);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments
//  AppendixB
vec3 BRDF_specularGGX(vec3 F0, vec3 F90, float alphaRoughness,
                      float specularWeight, float VdotH, float NdotL,
                      float NdotV, float NdotH) {
  const vec3 F = F_Schlick(F0, F90, VdotH);
  const float Vis = V_GGX(NdotL, NdotV, alphaRoughness);
  const float D = D_GGX(NdotH, alphaRoughness);
  return specularWeight * F * Vis * D;
}

#endif
