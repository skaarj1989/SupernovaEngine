#pragma once

float reinhard(float x) { return x / (1.0 + x); }

float3 reinhard2(float3 x, float maxWhite) {
  return (x * (1.0 + x / (maxWhite * maxWhite))) / (1.0 + x);
}
