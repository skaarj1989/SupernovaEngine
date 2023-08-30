#ifndef _COLOR_GLSL_
#define _COLOR_GLSL_

const float kGamma = 2.4;
const float kInvGamma = 1.0 / kGamma;

vec3 linearTosRGB(vec3 color) {
#if 1
  const bvec3 cutoff = lessThan(color, vec3(0.0031308));
  const vec3 higher = 1.055 * pow(color, vec3(kInvGamma)) - 0.055;
  const vec3 lower = color * 12.92;
  return mix(higher, lower, cutoff);
#else
  return pow(color, vec3(kInvGamma));
#endif
}
vec4 linearTosRGB(vec4 color) { return vec4(linearTosRGB(color.rgb), color.a); }
vec3 sRGBToLinear(vec3 color) {
#if 1
  const bvec3 cutoff = lessThan(color, vec3(0.04045));
  const vec3 higher = pow((color + 0.055) / 1.055, vec3(kGamma));
  const vec3 lower = color / 12.92;
  return mix(higher, lower, cutoff);
#else
  return vec3(pow(color, vec3(kGamma)));
#endif
}
vec4 sRGBToLinear(vec4 color) { return vec4(sRGBToLinear(color.rgb), color.a); }

float getLuminance(vec3 rgb) { return dot(rgb, vec3(0.2126, 0.7152, 0.0722)); }

vec3 RGB2XYZ(vec3 rgb) {
  // Reference:
  // RGB/XYZ Matrices
  // http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
  // clang-format off
  return mat3(
    0.4124564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,  
    0.1804375, 0.0721750, 0.9503041
  ) * rgb;
  // clang-format on
}
vec3 XYZ2RGB(vec3 xyz) {
  // clang-format off
  return mat3(
    3.2404542,-0.9692660, 0.0556434,
   -1.5371385, 1.8760108,-0.2040259,
   -0.4985314, 0.0415560, 1.0572252
  ) * xyz;
  // clang-format on
}

vec3 XYZ2Yxy(vec3 xyz) {
  // Reference:
  // http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
  const float inv = 1.0 / dot(xyz, vec3(1.0, 1.0, 1.0));
  return vec3(xyz.y, xyz.x * inv, xyz.y * inv);
}
vec3 Yxy2XYZ(vec3 Yxy) {
  // Reference:
  // http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
  vec3 xyz;
  xyz.x = Yxy.x * Yxy.y / Yxy.z;
  xyz.y = Yxy.x;
  xyz.z = Yxy.x * (1.0 - Yxy.y - Yxy.z) / Yxy.z;
  return xyz;
}

vec3 RGB2Yxy(vec3 rgb) { return XYZ2Yxy(RGB2XYZ(rgb)); }
vec3 Yxy2RGB(vec3 Yxy) { return XYZ2RGB(Yxy2XYZ(Yxy)); }

#endif
