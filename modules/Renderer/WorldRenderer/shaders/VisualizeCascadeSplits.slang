//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

#include "Resources/CameraBlock.h.slang"
#include "Resources/ShadowBlock.h.slang"

[[vk::binding(1, 2)]]
Texture2D t_SceneDepth : register(t0);

#include "Lib/Depth.h.slang"

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  const float depth = fetchDepth(t_SceneDepth, int2(input.position.xy));
  if (depth >= 1.0) discard; // Don't touch the far plane

  constexpr float kOverlayIntensity = 0.35;
  float4 fragColor = float4(0.0, 0.0, 0.0, kOverlayIntensity);

  const float3 fragPosViewSpace = viewPositionFromDepth(depth, input.texCoord);
  switch (_selectCascadeIndex(fragPosViewSpace)) {
  case 0:
    fragColor.rgb = float3(1.00, 0.25, 0.25);
    break;
  case 1:
    fragColor.rgb = float3(0.25, 1.00, 0.25);
    break;
  case 2:
    fragColor.rgb = float3(0.25, 0.25, 1.00);
    break;
  case 3:
    fragColor.rgb = float3(1.00, 1.00, 0.25);
    break;
  }
  return fragColor;
}
