//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

struct PushConstants {
  float radius;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(0, 0)]]
Sampler2D t_0;

[shader("fragment")]
float3 fragmentMain(VSOutput input) : SV_Target {
#define SAMPLE(x, y)                                                           \
  t_0.Sample(float2(x, y) * float2(u_PC.radius) + input.texCoord).rgb

  // Take 9 samples around current (e) texel:
  // a - b - c
  // d - e - f
  // g - h - i

  const float3 a = SAMPLE(-1.0, 1.0);
  const float3 b = SAMPLE(0.0, 1.0);
  const float3 c = SAMPLE(1.0, 1.0);

  const float3 d = SAMPLE(-1.0, 0.0);
  const float3 e = SAMPLE(0.0, 0.0);
  const float3 f = SAMPLE(1.0, 0.0);

  const float3 g = SAMPLE(-1.0, -1.0);
  const float3 h = SAMPLE(0.0, -1.0);
  const float3 i = SAMPLE(1.0, -1.0);

  // Apply weighted distribution, by using a 3x3 tent filter:
  //  1   | 1 2 1 |
  // -- * | 2 4 2 |
  // 16   | 1 2 1 |
  float3 fragColor = e * 4.0;
  fragColor += (b + d + f + h) * 2.0;
  fragColor += (a + c + g + i);
  fragColor *= 1.0 / 16.0;
  return fragColor;
}
