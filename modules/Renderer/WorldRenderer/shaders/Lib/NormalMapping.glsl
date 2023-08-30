#ifndef _NORMAL_MAPPING_GLSL
#define _NORMAL_MAPPING_GLSL

#include "CotangentFrame.glsl"

vec3 sampleNormalMap(sampler2D normalMap, vec2 texCoord) {
  return normalize(texture(normalMap, texCoord).rgb * 2.0 - 1.0);
}

// http://jbit.net/~sparky/sfgrad_bump/mm_sfgrad_bump.pdf

vec2 evaluateHeight(sampler2D bumpMap, vec2 texCoord, float scale) {
  const vec2 duv1 = dFdx(texCoord);
  const vec2 duv2 = dFdy(texCoord);
  const float Hll = texture(bumpMap, texCoord).r * scale;
  const float Hlr = texture(bumpMap, texCoord + duv1).r * scale;
  const float Hul = texture(bumpMap, texCoord + duv2).r * scale;
  return vec2(Hlr - Hll, Hul - Hll);
}

vec3 perturbNormal(vec3 N, vec2 dB) {
  const vec3 V = getViewDir();
  const vec3 sigmaS = dFdx(-V);
  const vec3 sigmaT = dFdy(-V);

  const vec3 R1 = cross(sigmaT, N);
  const vec3 R2 = cross(N, sigmaS);
  const float det = dot(sigmaS, R1);
  const vec3 surfaceGradient = sign(det) * (dB.s * R1 + dB.t * R2);
  return normalize(abs(det) * N - surfaceGradient);
}

vec3 tangentToWorld(vec3 v, vec3 N, vec2 uv) {
#ifdef HAS_NORMAL
#  ifdef HAS_TANGENTS
  return normalize(fs_in.TBN * v);
#  else
  const mat3 TBN = cotangentFrame(N, -getViewDir(), uv);
  return normalize(TBN * v);
#  endif
#else
  return vec3(0.0);
#endif
}
vec3 worldToTangent(vec3 v, vec3 N, vec2 uv) {
#ifdef HAS_NORMAL
#  ifdef HAS_TANGENTS
  return normalize(transpose(fs_in.TBN) * v);
#  else
  const mat3 TBN = cotangentFrame(N, -getViewDir(), uv);
  return normalize(transpose(TBN) * v);
#  endif
#else
  return vec3(0.0);
#endif
}

vec2 parallaxMapping(sampler2D heightMap, vec2 texCoord, vec3 V, vec3 N,
                     float scale) {
  const float minLayers = 8.0;
  const float maxLayers = 32.0;

  V = worldToTangent(-V, N, texCoord);

  const float numLayers =
    mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V)));
  const float layerDepth = 1.0 / numLayers;
  const vec2 P = V.xy / V.z * scale;
  const vec2 deltaTexCoords = P / numLayers;

  float currentDepthMapValue = textureLod(heightMap, texCoord, 0.0).r;

  float currentLayerDepth = 0.0;
  while (currentLayerDepth < currentDepthMapValue) {
    texCoord -= deltaTexCoords;
    currentDepthMapValue = texture(heightMap, texCoord).r;
    currentLayerDepth += layerDepth;
  }

  const vec2 prevTexCoords = texCoord + deltaTexCoords;

  const float afterDepth = currentDepthMapValue - currentLayerDepth;
  const float beforeDepth = textureLod(heightMap, prevTexCoords, 0.0).r -
                            currentLayerDepth + layerDepth;

  const float weight = afterDepth / (afterDepth - beforeDepth);
  return prevTexCoords * weight + texCoord * (1.0 - weight);
}

#endif
