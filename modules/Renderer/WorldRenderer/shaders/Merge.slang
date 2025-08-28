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
Texture2D t_0[NUM_TEXTURES];

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  float4 color = float4(0.0);
  [[unroll]]
  for (int i = 0; i < NUM_TEXTURES; ++i) {
    color += t_0[i].Sample(s_0, input.texCoord);
  }
  return color;
}
