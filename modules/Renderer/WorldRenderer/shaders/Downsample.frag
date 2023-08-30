#version 460 core
#extension GL_EXT_control_flow_attributes : require

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler2D t_0;

layout(push_constant) uniform _PushConstants { uint u_MipLevel; };

#include "Lib/Math.glsl"
#include "Lib/Color.glsl"
#include "Lib/Texture.glsl"

float karisAverage(vec3 color) {
  const float luma = getLuminance(linearTosRGB(color));
  return 1.0 / (1.0 + luma);
}

layout(location = 0) out vec3 FragColor;
void main() {
  const vec2 srcTexelSize = getTexelSize(t_0);
#define SAMPLE(x, y) texture(t_0, vec2(x, y) * srcTexelSize + v_TexCoord).rgb

  // Take 13 samples around current (e) texel:
  // a - b - c
  // - j - k -
  // d - e - f
  // - l - m -
  // g - h - i

  const vec3 a = SAMPLE(-2.0, 2.0);
  const vec3 b = SAMPLE(0.0, 2.0);
  const vec3 c = SAMPLE(2.0, 2.0);

  const vec3 j = SAMPLE(-1.0, 1.0);
  const vec3 k = SAMPLE(1.0, 1.0);

  const vec3 d = SAMPLE(-2.0, 0.0);
  const vec3 e = SAMPLE(0.0, 0.0);
  const vec3 f = SAMPLE(2.0, 0.0);

  const vec3 l = SAMPLE(-1.0, -1.0);
  const vec3 m = SAMPLE(1.0, -1.0);

  const vec3 g = SAMPLE(-2.0, -2.0);
  const vec3 h = SAMPLE(0.0, -2.0);
  const vec3 i = SAMPLE(2.0, -2.0);

  const int kNumGroups = 5;
  vec3 groups[kNumGroups];
  if (u_MipLevel == 0) {
    groups[0] = (a + b + d + e) * (0.125 / 4.0);
    groups[1] = (b + c + e + f) * (0.125 / 4.0);
    groups[2] = (d + e + g + h) * (0.125 / 4.0);
    groups[3] = (e + f + h + i) * (0.125 / 4.0);
    groups[4] = (j + k + l + m) * (0.5 / 4.0);
    [[unroll]] for (int i = 0; i < kNumGroups; ++i) {
      groups[i] *= karisAverage(groups[i]);
    }
    FragColor = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
    FragColor = max(FragColor, EPSILON);
  } else {
    FragColor = e * 0.125;
    FragColor += (a + c + g + i) * 0.03125;
    FragColor += (b + d + f + h) * 0.0625;
    FragColor += (j + k + l + m) * 0.125;
  }
}
