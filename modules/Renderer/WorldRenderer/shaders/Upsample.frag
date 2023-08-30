#version 460 core

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler2D t_0;

layout(push_constant) uniform _PushConstants { float u_Radius; };

layout(location = 0) out vec3 FragColor;
void main() {
#define SAMPLE(x, y) texture(t_0, vec2(x, y) * vec2(u_Radius) + v_TexCoord).rgb

  // Take 9 samples around current (e) texel:
  // a - b - c
  // d - e - f
  // g - h - i

  const vec3 a = SAMPLE(-1.0, 1.0);
  const vec3 b = SAMPLE(0.0, 1.0);
  const vec3 c = SAMPLE(1.0, 1.0);

  const vec3 d = SAMPLE(-1.0, 0.0);
  const vec3 e = SAMPLE(0.0, 0.0);
  const vec3 f = SAMPLE(1.0, 0.0);

  const vec3 g = SAMPLE(-1.0, -1.0);
  const vec3 h = SAMPLE(0.0, -1.0);
  const vec3 i = SAMPLE(1.0, -1.0);

  // Apply weighted distribution, by using a 3x3 tent filter:
  //  1   | 1 2 1 |
  // -- * | 2 4 2 |
  // 16   | 1 2 1 |
  FragColor = e * 4.0;
  FragColor += (b + d + f + h) * 2.0;
  FragColor += (a + c + g + i);
  FragColor *= 1.0 / 16.0;
}
