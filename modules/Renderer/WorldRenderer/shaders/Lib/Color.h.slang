#pragma once

static const float kGamma = 2.4;
static const float kInvGamma = 1.0 / kGamma;

float3 linearTosRGB(float3 color) {
#if 1
  const bool3 cutoff = color < float3(0.0031308);
  const float3 higher = 1.055 * pow(color, float3(kInvGamma)) - 0.055;
  const float3 lower = color * 12.92;
  return lerp(higher, lower, float3(cutoff));
#else
  return pow(color, float3(kInvGamma));
#endif
}
float4 linearTosRGB(float4 color) {
  return float4(linearTosRGB(color.rgb), color.a);
}
float3 sRGBToLinear(float3 color) {
#if 1
  const bool3 cutoff = color < float3(0.04045);
  const float3 higher = pow((color + 0.055) / 1.055, float3(kGamma));
  const float3 lower = color / 12.92;
  return lerp(higher, lower, float3(cutoff));
#else
  return float3(pow(color, float3(kGamma)));
#endif
}
float4 sRGBToLinear(float4 color) {
  return float4(sRGBToLinear(color.rgb), color.a);
}

float getLuminance(float3 rgb) {
  return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}

float3 RGB2XYZ(float3 rgb) {
  // Reference:
  // RGB/XYZ Matrices
  // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
  // clang-format off
  return mul(float3x3(
    0.4124564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,  
    0.1804375, 0.0721750, 0.9503041
  ), rgb);
  // clang-format on
}
float3 XYZ2RGB(float3 xyz) {
  // clang-format off
  return mul(float3x3(
    3.2404542,-0.9692660, 0.0556434,
   -1.5371385, 1.8760108,-0.2040259,
   -0.4985314, 0.0415560, 1.0572252
  ), xyz);
  // clang-format on
}

float3 XYZ2Yxy(float3 xyz) {
  // Reference:
  // http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
  const float inv = 1.0 / dot(xyz, float3(1.0, 1.0, 1.0));
  return float3(xyz.y, xyz.x * inv, xyz.y * inv);
}
float3 Yxy2XYZ(float3 Yxy) {
  // Reference:
  // http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
  float3 xyz;
  xyz.x = Yxy.x * Yxy.y / Yxy.z;
  xyz.y = Yxy.x;
  xyz.z = Yxy.x * (1.0 - Yxy.y - Yxy.z) / Yxy.z;
  return xyz;
}

float3 RGB2Yxy(float3 rgb) { return XYZ2Yxy(RGB2XYZ(rgb)); }
float3 Yxy2RGB(float3 Yxy) { return XYZ2RGB(Yxy2XYZ(Yxy)); }
