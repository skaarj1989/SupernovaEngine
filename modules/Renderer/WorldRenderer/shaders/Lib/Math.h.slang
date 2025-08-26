#pragma once

// -- CONSTANTS

static const float PI = 3.1415926535897932384626433832795;
static const float TAU = 6.2831853071795864769252867665590;
static const float HALF_PI = 1.5707963267948966192313216916398;

static const float EPSILON = 1.19e-07;

// -- FUNCTIONS

#define rcp(v) (1.0 / v)
#define sq(v) (v * v)

#define clamp01(x) clamp(x, 0.0, 1.0)

float max3(float3 v) { return max(max(v.x, v.y), v.z); }

float inverselerp(float x, float y, float v) { return (v - x) / (y - x); }
float4 inverselerp(float4 x, float4 y, float4 v) { return (v - x) / (y - x); }

// Remap value v from input range i to output range o
float remap(float v, float2 i, float2 o) {
  const float a = inverselerp(i.x, i.y, v);
  return lerp(o.x, o.y, a);
}
float2 remap(float2 v, float2 i, float2 o) {
  return float2(remap(v.x, i, o), remap(v.y, i, o));
}
float3 remap(float3 v, float2 i, float2 o) {
  return float3(remap(v.xy, i, o), remap(v.z, i, o));
}
float4 remap(float4 v, float2 i, float2 o) {
  return float4(remap(v.xyz, i, o), remap(v.w, i, o));
}

bool isApproximatelyEqual(float a, float b) {
  return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// [0..1]
float rand(float2 co) {
  const float a = 12.9898;
  const float b = 78.233;
  const float c = 43758.5453;
  const float dt = dot(co, float2(a, b));
  const float sn = fmod(dt, PI);
  return frac(sin(sn) * c);
}

// [-1..1]
float srand(float2 co) { return rand(co) * 2.0 - 1.0; }

// Generic function generator
#define _GEN_TYPE_F(name, arg, expr)                                           \
  float name(float arg) { return expr; }                                       \
  float2 name(float2 arg) { return expr; }                                     \
  float3 name(float3 arg) { return expr; }                                     \
  float4 name(float4 arg) { return expr; }

_GEN_TYPE_F(mod7, x, x - floor(x * (1.0 / 7.0)) * 7.0)
_GEN_TYPE_F(mod289, x, x - floor(x * (1.0 / 289.0)) * 289.0)
_GEN_TYPE_F(permute, x, mod289((34.0 * x + 10.0) * x))
_GEN_TYPE_F(taylorInvSqrt, r, 1.79284291400159 - 0.85373472095314 * r)
_GEN_TYPE_F(fade, t, t *t *t *(t *(t * 6.0 - 15.0) + 10.0))

// Flatten/unflatten utilities
uint flatten2D(uint2 id, uint width) { return id.x + width * id.y; }
uint2 unflatten2D(uint i, uint width) { return uint2(i % width, i / width); }

uint flatten3D(uint3 id, uint2 size) {
  return id.x + size.x * id.y + size.x * size.y * id.z;
}
uint3 unflatten3D(uint i, uint2 size) {
  return uint3(i % size.x, (i / size.x) % size.y, i / (size.x * size.y));
}
