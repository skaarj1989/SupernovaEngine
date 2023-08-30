#ifndef _OMNI_SHADOW_GLSL_
#define _OMNI_SHADOW_GLSL_

float _vectorToDepthValue(float n, float f, vec3 v) {
  // https://stackoverflow.com/a/23445511
  const float localZ = -max3(abs(v));
  const float m23 = -1.0;

#if DEPTH_ZERO_TO_ONE
  const float m22 = f / (n - f);
  const float m32 = -(f * n) / (f - n);
#else
  const float m22 = -(f + n) / (f - n);
  const float m32 = -(2.0 * f * n) / (f - n);
#endif
  const float normZ = (localZ * m22 + m32) / (localZ * m23);

#if DEPTH_ZERO_TO_ONE
  return normZ;
#else
  return normZ * 0.5 + 0.5;
#endif
}

float _getPointLightVisibility(textureCubeArray shadowMaps,
                               samplerShadow shadowSampler,
                               const in Light light, vec3 fragPosWorldSpace,
                               float NdotL) {
  const float lightRange = _getLightRange(light);
  const float bias = _getShadowBias(light);

  const vec3 L = fragPosWorldSpace - light.position.xyz;
  const float d = _vectorToDepthValue(0.1, lightRange, L);

#if !SOFT_SHADOWS
  return texture(samplerCubeArrayShadow(shadowMaps, shadowSampler),
                 vec4(L, light.shadowMapId), d - bias);
#else
  const float kSamples = 4.0;
  const float kOffset = 0.1;
  const float kStep = kOffset / (kSamples * 0.5);

  float shadowFactor = 0.0;
  uint count = 0;
  for (float x = -kOffset; x < kOffset; x += kStep) {
    for (float y = -kOffset; y < kOffset; y += kStep) {
      for (float z = -kOffset; z < kOffset; z += kStep) {
        shadowFactor +=
          texture(samplerCubeArrayShadow(shadowMaps, shadowSampler),
                  vec4(L + vec3(x, y, z), light.shadowMapId), d - bias);
        ++count;
      }
    }
  }
  return shadowFactor / float(count);
#endif
}

#endif
