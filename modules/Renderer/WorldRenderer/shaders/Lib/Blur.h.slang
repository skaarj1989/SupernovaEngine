#pragma once

// https://github.com/Jam3/glsl-fast-gaussian-blur

float4 blur5(Sampler2D image, float2 uv, float2 resolution, float2 direction) {
  float4 color = float4(0.0);
  const float2 off1 = float2(1.3333333333333333) * direction;
  color += image.Sample(uv) * 0.29411764705882354;
  color += image.Sample(uv + (off1 / resolution)) * 0.35294117647058826;
  color += image.Sample(uv - (off1 / resolution)) * 0.35294117647058826;
  return color;
}

float4 blur9(Sampler2D image, float2 uv, float2 resolution, float2 direction) {
  float4 color = float4(0.0);
  const float2 off1 = float2(1.3846153846) * direction;
  const float2 off2 = float2(3.2307692308) * direction;
  color += image.Sample(uv) * 0.2270270270;
  color += image.Sample(uv + (off1 / resolution)) * 0.3162162162;
  color += image.Sample(uv - (off1 / resolution)) * 0.3162162162;
  color += image.Sample(uv + (off2 / resolution)) * 0.0702702703;
  color += image.Sample(uv - (off2 / resolution)) * 0.0702702703;
  return color;
}

float4 blur13(Sampler2D image, float2 uv, float2 resolution, float2 direction) {
  float4 color = float4(0.0);
  const float2 off1 = float2(1.411764705882353) * direction;
  const float2 off2 = float2(3.2941176470588234) * direction;
  const float2 off3 = float2(5.176470588235294) * direction;
  color += image.Sample(uv) * 0.1964825501511404;
  color += image.Sample(uv + (off1 / resolution)) * 0.2969069646728344;
  color += image.Sample(uv - (off1 / resolution)) * 0.2969069646728344;
  color += image.Sample(uv + (off2 / resolution)) * 0.09447039785044732;
  color += image.Sample(uv - (off2 / resolution)) * 0.09447039785044732;
  color += image.Sample(uv + (off3 / resolution)) * 0.010381362401148057;
  color += image.Sample(uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}
