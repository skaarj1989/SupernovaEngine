#version 460 core

layout(location = 0) in FragData {
  vec3 N;
  vec4 flux;
}
fs_in;

#include "Lib/Math.glsl"
#include "Lib/LPV.glsl"

layout(location = 0) out vec4 SH_R;
layout(location = 1) out vec4 SH_G;
layout(location = 2) out vec4 SH_B;

void main() {
  const vec4 SH_coeffs = SH_evaluateCosineLobe(fs_in.N) / PI;
  SH_R = SH_coeffs * fs_in.flux.r;
  SH_G = SH_coeffs * fs_in.flux.g;
  SH_B = SH_coeffs * fs_in.flux.b;
}
