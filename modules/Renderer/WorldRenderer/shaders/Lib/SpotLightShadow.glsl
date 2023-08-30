#ifndef _SPOTLIGHT_SHADOW_GLSL_
#define _SPOTLIGHT_SHADOW_GLSL_

float _getSpotLightVisibility(texture2DArray shadowMaps,
                              samplerShadow shadowSampler, const in Light light,
                              vec3 fragPosWorldSpace, float NdotL) {
  vec4 shadowCoord = u_ShadowBlock.spotLightsViewProj[light.shadowMapId] *
                     vec4(fragPosWorldSpace, 1.0);
  shadowCoord = shadowCoord / shadowCoord.w;

  const float bias = _getShadowBias(light);
#if !SOFT_SHADOWS
  return texture(sampler2DArrayShadow(shadowMaps, shadowSampler),
                 vec4(shadowCoord.xy, light.shadowMapId, shadowCoord.z - bias));
#else
  const ivec2 shadowMapSize = textureSize(shadowMaps, 0).xy;
  const float kScale = 1.0;
  const float dx = kScale * 1.0 / float(shadowMapSize.x);
  const float dy = kScale * 1.0 / float(shadowMapSize.y);

  const int kRange = 2;
  float shadowFactor = 0.0;
  uint count = 0;
  for (int x = -kRange; x <= kRange; ++x) {
    for (int y = -kRange; y <= kRange; ++y) {
      shadowFactor += texture(sampler2DArrayShadow(shadowMaps, shadowSampler),
                              vec4(shadowCoord.xy + vec2(dx * x, dy * y),
                                   light.shadowMapId, shadowCoord.z - bias));
      ++count;
    }
  }
  return shadowFactor / count;
#endif
}

#endif
