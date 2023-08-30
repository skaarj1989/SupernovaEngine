#ifndef _COMMON_GLSL_
#define _COMMON_GLSL_

#include "Math.glsl"

#ifdef GL_FRAGMENT_SHADER
// @return .xy = [0..1]
vec4 getScreenCoord() {
  return vec4(gl_FragCoord.xy / getResolution(), gl_FragCoord.zw);
}
#endif

// @return Direction to camera
#define getViewDir() normalize(getCameraPosition() - getPosition().xyz)

float fresnelEffect(vec3 N, vec3 V, float power) {
  return pow((1.0 - clamp01(dot(normalize(N), V))), power);
}

vec2 tileAndOffset(vec2 texCoord, vec2 tiling, float offset) {
  return texCoord * tiling + offset;
}

vec2 flip(vec2 v, bvec2 flip_) { return (vec2(flip_) * -2.0 + 1.0) * v; }
vec3 flip(vec3 v, bvec3 flip_) { return (vec3(flip_) * -2.0 + 1.0) * v; }
vec4 flip(vec4 v, bvec4 flip_) { return (vec4(flip_) * -2.0 + 1.0) * v; }

#endif
