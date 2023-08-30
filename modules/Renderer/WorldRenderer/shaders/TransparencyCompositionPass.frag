#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 1) uniform texture2D t_Accum;
layout(set = 0, binding = 2) uniform texture2D t_Reveal;

#include "Lib/Math.glsl"

layout(location = 0) out vec4 FragColor;
void main() {
  const ivec2 coords = ivec2(gl_FragCoord.xy);
  const float revealage = texelFetch(t_Reveal, coords, 0).r;

  // Save the blending and color texture fetch cost if there is not a
  // transparent fragment
  if (isApproximatelyEqual(revealage, 1.0)) discard;

  vec4 accumulation = texelFetch(t_Accum, coords, 0);
  // Suppress overflow
  if (isinf(max3(abs(accumulation.rgb))))
    accumulation.rgb = vec3(accumulation.a);

  const vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);
  FragColor = vec4(averageColor, 1.0 - revealage);
}
