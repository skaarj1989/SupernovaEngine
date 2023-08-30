#ifndef _DEPTH_GLSL_
#define _DEPTH_GLSL_

#ifndef DEPTH_ZERO_TO_ONE
#  error "Check preamble"
#endif

#if DEPTH_ZERO_TO_ONE
#  define NEAR_CLIP_PLANE 0.0
#else
#  define NEAR_CLIP_PLANE -1.0
#endif

#include "Math.glsl"
#include "SpaceTraversal.glsl"

// Returns depth value in clip-space
#if DEPTH_ZERO_TO_ONE
#  define getDepth(s, uv) texture(s, uv).r
#  define fetchDepth(s, coord) texelFetch(s, coord, 0).r
#else
#  define getDepth(s, uv) texture(s, uv).r * 2.0 - 1.0
#  define fetchDepth(s, coord) texelFetch(s, coord, 0).r * 2.0 - 1.0
#endif

float linearizeDepth(float n, float f, float sampledDepth) {
#if DEPTH_ZERO_TO_ONE
  const float z = sampledDepth;
#else
  const float z = sampledDepth * 2.0 - 1.0;
#endif
  return n * f / (f + z * (n - f));
}

float linearizeDepth(float sampledDepth) {
  return linearizeDepth(u_Camera.near, u_Camera.far, sampledDepth);
}

// @param z in NDC
// @param uv in range: [0..1]
vec3 viewPositionFromDepth(float z, vec2 uv) {
  // https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945
  const vec4 ndc = vec4(uv * 2.0 - 1.0, z, 1.0);
  return NDCToView(ndc);
}

#endif
