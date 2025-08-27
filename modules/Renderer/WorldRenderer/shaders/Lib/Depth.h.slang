#pragma once

#ifndef DEPTH_ZERO_TO_ONE
#  error "Check preamble"
#endif

#if DEPTH_ZERO_TO_ONE
#  define NEAR_CLIP_PLANE 0.0
#else
#  define NEAR_CLIP_PLANE -1.0
#endif

#include "Math.h.slang"
#include "SpaceTraversal.h.slang"

// Returns depth value in clip-space
#if DEPTH_ZERO_TO_ONE
#  define getDepth(s, uv) s.Sample(sampler, uv).r
#  define fetchDepth(s, coord) s.Load(int3(coord, 0)).r
#else
#  define getDepth(s, uv) s.Sample(sampler, uv).r * 2.0 - 1.0
#  define fetchDepth(s, coord) s.Load(int3(coord, 0)).r * 2.0 - 1.0
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
// @param uv in range [0..1]
float3 viewPositionFromDepth(float z, float2 uv) {
  // https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945
  const float4 ndc = float4(uv * 2.0 - 1.0, z, 1.0);
  return NDCToView(ndc);
}
