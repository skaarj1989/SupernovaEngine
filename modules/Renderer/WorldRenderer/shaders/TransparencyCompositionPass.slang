//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

[[vk::binding(1, 0)]]
Texture2D t_Accum;
[[vk::binding(2, 0)]]
Texture2D t_Reveal;

#include "Lib/Math.h.slang"

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  const int2 coords = int2(input.position.xy);
  const float revealage = t_Reveal.Load(int3(coords, 0)).r;

  // Save the blending and color texture fetch cost if there is not a
  // transparent fragment
  if (isApproximatelyEqual(revealage, 1.0)) discard;

  float4 accumulation = t_Accum.Load(int3(coords, 0));
  // Suppress overflow
  if (isinf(max3(abs(accumulation.rgb))))
    accumulation.rgb = float3(accumulation.a);

  const float3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);
  return float4(averageColor, 1.0 - revealage);
}
