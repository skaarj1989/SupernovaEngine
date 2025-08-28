#pragma once

float3 partialTonemapUncharted2(float3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
float3 tonemapUncharted2(float3 v) {
  const float exposureBias = 2.0;
  float3 curr = partialTonemapUncharted2(v * exposureBias);

  float3 W = float3(11.2);
  const float3 whiteScale = float3(1.0) / partialTonemapUncharted2(W);
  return curr * whiteScale;
}
