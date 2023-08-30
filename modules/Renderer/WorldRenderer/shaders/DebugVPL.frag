#version 460 core

layout(location = 0) in FragData {
  vec3 cellCoord;
  vec3 N;
}
fs_in;

layout(set = 0, binding = 3) uniform sampler s_0;

layout(set = 2, binding = 3) uniform texture3D t_AccumulatedSH_R;
layout(set = 2, binding = 4) uniform texture3D t_AccumulatedSH_G;
layout(set = 2, binding = 5) uniform texture3D t_AccumulatedSH_B;

#include "Lib/LPV.glsl"

layout(location = 0) out vec4 FragColor;

void main() {
  const vec4 SH_intensity = SH_evaluate(normalize(fs_in.N));

  // clang-format off
  const SHcoeffs coeffs = {
    texture(sampler3D(t_AccumulatedSH_R, s_0), fs_in.cellCoord, 0),
    texture(sampler3D(t_AccumulatedSH_G, s_0), fs_in.cellCoord, 0),
    texture(sampler3D(t_AccumulatedSH_B, s_0), fs_in.cellCoord, 0)
  };
  // clang-format on
  const vec3 intensity =
    vec3(dot(SH_intensity, coeffs.red), dot(SH_intensity, coeffs.green),
         dot(SH_intensity, coeffs.blue));

  FragColor.rgb = intensity;
}
