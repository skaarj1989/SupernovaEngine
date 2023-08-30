#ifndef _SAMPLE_SPHERICAL_MAP_GLSL_
#define _SAMPLE_SPHERICAL_MAP_GLSL_

#include "Math.glsl"

// @param dir must be normalized
vec2 sampleSphericalMap(vec3 dir) {
  vec2 v = vec2(atan(dir.z, dir.x), asin(dir.y));
  v *= vec2(1.0 / TAU, 1.0 / PI); // -> [-0.5, 0.5]
  return v + 0.5;                 // -> [0.0, 1.0]
}

#endif
