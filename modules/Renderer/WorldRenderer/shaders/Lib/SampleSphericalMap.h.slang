#pragma once

#include "Math.h.slang"

// @param dir must be normalized
float2 sampleSphericalMap(float3 dir) {
  float2 v = float2(atan2(dir.z, dir.x), asin(dir.y));
  v *= float2(1.0 / TAU, 1.0 / PI); // -> [-0.5, 0.5]
  return v + 0.5;                   // -> [0.0, 1.0]
}
