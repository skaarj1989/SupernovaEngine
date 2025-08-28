//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

[[vk::binding(0, 0)]]
SamplerState s_0;

[[vk::binding(1, 0)]]
Texture2D t_A;
[[vk::binding(2, 0)]]
Texture2D t_B;

#include "Lib/Math.h.slang"

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  const float4 a = t_A.Sample(s_0, input.texCoord);
  const float4 b = t_B.Sample(s_0, input.texCoord);
  return float4(lerp(a.rgb, b.rgb, clamp01(b.a)), 1.0);
}
