#ifndef _CSM_GLSL_
#define _CSM_GLSL_

float _getDirLightVisibility(uint cascadeIndex,
                             texture2DArray cascadedShadowMaps,
                             samplerShadow shadowSampler, const in Light light,
                             vec3 fragPosWorldSpace, float NdotL) {
  vec4 shadowCoord = u_ShadowBlock.cascades.viewProjMatrices[cascadeIndex] *
                     vec4(fragPosWorldSpace, 1.0);
  shadowCoord.xyz /= shadowCoord.w;

  const float bias = _getShadowBias(light);

#if !SOFT_SHADOWS
  return texture(sampler2DArrayShadow(cascadedShadowMaps, shadowSampler),
                 vec4(shadowCoord.xy, cascadeIndex, shadowCoord.z - bias));
#else
  const ivec2 shadowMapSize = textureSize(cascadedShadowMaps, 0).xy;
  const float kScale = 1.0;
  const float dx = kScale * 1.0 / float(shadowMapSize.x);
  const float dy = kScale * 1.0 / float(shadowMapSize.y);

  const int kRange = 1;
  float shadowFactor = 0.0;
  uint count = 0;
  for (int x = -kRange; x <= kRange; ++x) {
    for (int y = -kRange; y <= kRange; ++y) {
      shadowFactor +=
        texture(sampler2DArrayShadow(cascadedShadowMaps, shadowSampler),
                vec4(shadowCoord.xy + vec2(dx * x, dy * y), cascadeIndex,
                     shadowCoord.z - bias));
      ++count;
    }
  }
  return shadowFactor / float(count);
#endif
}

#endif
