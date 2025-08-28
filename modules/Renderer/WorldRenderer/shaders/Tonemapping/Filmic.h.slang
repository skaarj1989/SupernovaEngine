#pragma once

float3 tonemapFilmic(float3 x) {
  float3 X = max(float3(0.0), x - 0.004);
  float3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
  return pow(result, float3(2.2));
}
