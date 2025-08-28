//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

#include "Resources/CameraBlock.h.slang"
#include "Lib/Depth.h.slang"
#include "Lib/Color.h.slang"

enum Mode : uint {
  Default = 0,
  LinearDepth,
  RedChannel,
  GreenChannel,
  BlueChannel,
  AlphaChannel,
  ViewSpaceNormals,
  WorldSpaceNormals,
};

struct PushConstants {
  Mode mode;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(1, 2)]]
Sampler2D t_0;

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  const float4 source = t_0.Sample(input.texCoord);

  float3 fragColor = float3(0, 0, 0);
  switch (u_PC.mode) {
  case Mode::LinearDepth:
    fragColor = float3(linearizeDepth(source.r) / u_Camera.far);
    break;
  case Mode::RedChannel:
    fragColor = source.rrr;
    break;
  case Mode::GreenChannel:
    fragColor = source.ggg;
    break;
  case Mode::BlueChannel:
    fragColor = source.bbb;
    break;
  case Mode::AlphaChannel:
    fragColor = source.aaa;
    break;

  case Mode::ViewSpaceNormals:
    fragColor = normalize(worldToView(float4(source.rgb, 0.0)));
    break;
  case Mode::WorldSpaceNormals:
    fragColor = normalize(source.rgb);
    break;

  case Mode::Default:
  default:
    fragColor = source.rgb;
    break;
  }
  return float4(fragColor, 1.0);
}
