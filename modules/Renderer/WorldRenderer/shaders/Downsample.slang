//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

struct PushConstants {
  uint mipLevel;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(0, 0)]]
Sampler2D t_0;

#include "Lib/Math.h.slang"
#include "Lib/Color.h.slang"
#include "Lib/Texture.h.slang"

float karisAverage(float3 color) {
  const float luma = getLuminance(linearTosRGB(color));
  return 1.0 / (1.0 + luma);
}

[shader("fragment")]
float3 fragmentMain(VSOutput input) : SV_Target {
  const float2 srcTexelSize = getTexelSize(t_0);
#define SAMPLE(x, y)                                                           \
  t_0.Sample(float2(x, y) * srcTexelSize + input.texCoord).rgb

  // Take 13 samples around current (e) texel:
  // a - b - c
  // - j - k -
  // d - e - f
  // - l - m -
  // g - h - i

  const float3 a = SAMPLE(-2.0, 2.0);
  const float3 b = SAMPLE(0.0, 2.0);
  const float3 c = SAMPLE(2.0, 2.0);

  const float3 j = SAMPLE(-1.0, 1.0);
  const float3 k = SAMPLE(1.0, 1.0);

  const float3 d = SAMPLE(-2.0, 0.0);
  const float3 e = SAMPLE(0.0, 0.0);
  const float3 f = SAMPLE(2.0, 0.0);

  const float3 l = SAMPLE(-1.0, -1.0);
  const float3 m = SAMPLE(1.0, -1.0);

  const float3 g = SAMPLE(-2.0, -2.0);
  const float3 h = SAMPLE(0.0, -2.0);
  const float3 i = SAMPLE(2.0, -2.0);

  float3 fragColor;

  const int kNumGroups = 5;
  float3 groups[kNumGroups];
  if (u_PC.mipLevel == 0) {
    groups[0] = (a + b + d + e) * (0.125 / 4.0);
    groups[1] = (b + c + e + f) * (0.125 / 4.0);
    groups[2] = (d + e + g + h) * (0.125 / 4.0);
    groups[3] = (e + f + h + i) * (0.125 / 4.0);
    groups[4] = (j + k + l + m) * (0.5 / 4.0);
    [[unroll]]
    for (int i = 0; i < kNumGroups; ++i) {
      groups[i] *= karisAverage(groups[i]);
    }
    fragColor = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
    fragColor = max(fragColor, EPSILON);
  } else {
    fragColor = e * 0.125;
    fragColor += (a + c + g + i) * 0.03125;
    fragColor += (b + d + f + h) * 0.0625;
    fragColor += (j + k + l + m) * 0.125;
  }
  return fragColor;
}
