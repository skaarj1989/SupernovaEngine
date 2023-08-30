#ifndef _COTANGENT_FRAME_GLSL_
#define _COTANGENT_FRAME_GLSL_

#if defined(HAS_NORMAL) && !defined(HAS_TANGENTS)

// http://www.thetenthplanet.de/archives/1180
// https://gamedev.stackexchange.com/questions/86530/is-it-possible-to-calculate-the-tbn-matrix-in-the-fragment-shader

mat3 cotangentFrame(vec3 N, vec3 p, vec2 uv) {
  const vec3 dp1 = dFdx(p);
  const vec3 dp2 = dFdy(p);

  const vec2 duv1 = dFdx(uv);
  const vec2 duv2 = dFdy(uv);

  const vec3 dp2perp = cross(dp2, N);
  const vec3 dp1perp = cross(N, dp1);
  const vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  const vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  const float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  return mat3(T * invmax, B * invmax, N);
}

#endif

#endif
