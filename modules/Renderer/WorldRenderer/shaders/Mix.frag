#version 460 core

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler s_0;
layout(set = 0, binding = 1) uniform texture2D t_A;
layout(set = 0, binding = 2) uniform texture2D t_B;

#include "Lib/Math.glsl"

layout(location = 0) out vec4 FragColor;
void main() {
  const vec4 a = texture(sampler2D(t_A, s_0), v_TexCoord);
  const vec4 b = texture(sampler2D(t_B, s_0), v_TexCoord);
  FragColor = vec4(mix(a.rgb, b.rgb, clamp01(b.a)), 1.0);
}
