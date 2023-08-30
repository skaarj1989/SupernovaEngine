#ifndef _MATH_GLSL
#define _MATH_GLSL

// -- CONSTANTS:

#define PI 3.1415926535897932384626433832795
#define TAU 6.2831853071795864769252867665590
#define HALF_PI 1.5707963267948966192313216916398

#define EPSILON 1.19e-07

// -- FUNCTIONS:

#define rcp(v) (1.0 / v)
#define sq(v) (v * v)

#define clamp01(x) clamp(x, 0.0, 1.0)

float max3(vec3 v) { return max(max(v.x, v.y), v.z); }

#define lerp mix
float inverselerp(float x, float y, float v) { return (v - x) / (y - x); }
vec4 inverselerp(vec4 x, vec4 y, vec4 v) { return (v - x) / (y - x); }

// @param v value to remap
// @param i input range [min, max]
// @param o output range [min, max]
float remap(float v, vec2 i, vec2 o) {
  const float a = inverselerp(i.x, i.y, v);
  return mix(o.x, o.y, a);
}
vec2 remap(vec2 v, vec2 i, vec2 o) {
  return vec2(remap(v.x, i, o), remap(v.y, i, o));
}
vec3 remap(vec3 v, vec2 i, vec2 o) {
  return vec3(remap(v.xy, i, o), remap(v.z, i, o));
}
vec4 remap(vec4 v, vec2 i, vec2 o) {
  return vec4(remap(v.xyz, i, o), remap(v.a, i, o));
}

bool isApproximatelyEqual(float a, float b) {
  return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// [0..1]
float rand(vec2 co) {
  // http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
  const float a = 12.9898;
  const float b = 78.233;
  const float c = 43758.5453;
  const float dt = dot(co, vec2(a, b));
  const float sn = mod(dt, PI);
  return fract(sin(sn) * c);
}
// [-1..1]
float srand(vec2 co) { return rand(co) * 2.0 - 1.0; }

#define _GEN_TYPE_F(name, arg, expr)                                           \
  float name(float arg) { return expr; }                                       \
  vec2 name(vec2 arg) { return expr; }                                         \
  vec3 name(vec3 arg) { return expr; }                                         \
  vec4 name(vec4 arg) { return expr; }

// clang-format off

// Modulo 7 without a division
_GEN_TYPE_F(mod7, x, x - floor(x * (1.0 / 7.0)) * 7.0)
// Modulo 289 without a division (only multiplications)
_GEN_TYPE_F(mod289, x, x - floor(x * (1.0 / 289.0)) * 289.0)
// Permutation polynomial: (34x^2 + 6x) mod 289
_GEN_TYPE_F(permute, x, mod289((34.0 * x + 10.0) * x))

_GEN_TYPE_F(taylorInvSqrt, r, 1.79284291400159 - 0.85373472095314 * r)
_GEN_TYPE_F(fade, t, t * t * t * (t * (t * 6.0 - 15.0) + 10.0))

// clang-format on

// https://softwareengineering.stackexchange.com/questions/212808/treating-a-1d-data-structure-as-2d-grid/212813

uint flatten2D(uvec2 id, uint width) { return id.x + width * id.y; }
uvec2 unflatten2D(uint i, uint width) { return uvec2(i % width, i / width); }

uint flatten3D(uvec3 id, uvec2 size) {
  return id.x + size.x * id.y + size.x * size.y * id.z;
}
uvec3 unflatten3D(uint i, uvec2 size) {
  return uvec3(i % size.x, (i / size.x) % size.y, i / (size.x * size.y));
}

#endif
