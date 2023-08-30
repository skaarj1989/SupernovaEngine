#ifndef _TONEMAP_ACES_GLSL_
#define _TONEMAP_ACES_GLSL_

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

#include <Lib/Math.glsl>

vec3 ACES(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp01((x * (a * x + b)) / (x * (c * x + d) + e));
}

#endif
