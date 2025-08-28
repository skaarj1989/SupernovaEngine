//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

#include "Lib/Texture.h.slang"
#include "Lib/Blur.h.slang"

struct PushConstants {
  float2 direction;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(1, 0)]]
Sampler2D t_0;

[shader("fragment")]
float3 fragmentMain(VSOutput input) : SV_Target {
  float2 resolution;
  t_0.GetDimensions(resolution.x, resolution.y);
  return blur5(t_0, input.texCoord, resolution, u_PC.direction).rgb;
}
