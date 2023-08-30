#ifndef _TONEMAP_REINHARD_GLSL_
#define _TONEMAP_REINHARD_GLSL_

float reinhard(float x) { return x / (1.0 + x); }

vec3 reinhard2(vec3 x, float maxWhite) {
  return (x * (1.0 + x / (maxWhite * maxWhite))) / (1.0 + x);
}

#endif
