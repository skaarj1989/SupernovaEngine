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
Texture2D t_SceneColor;

#if HAS_EYE_ADAPTATION
[[vk::binding(2, 0), vk::image_format("r16f")]]
Texture2D i_AverageLuminance;
#endif
#if HAS_BLOOM
[[vk::binding(3, 0)]]
Texture2D t_Bloom;
#endif

#include "Tonemapping/ACES.h.slang"
#include "Tonemapping/Filmic.h.slang"
#include "Tonemapping/Reinhard.h.slang"
#include "Tonemapping/Uncharted.h.slang"

enum TonemapMode : uint {
  Clamp = 0,
  ACES,
  Filmic,
  Reinhard,
  Uncharted,
};

struct PushConstants {
  TonemapMode tonemap;
  float exposure;
  float bloomStrength;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

#include "Lib/Math.h.slang"
#include "Lib/Color.h.slang"

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  float3 sceneColor = t_SceneColor.Sample(s_0, input.texCoord).rgb;

#if HAS_EYE_ADAPTATION
  const float luminance = i_AverageLuminance.Load(int3(0, 0, 0)).r;
  float3 Yxy = RGB2Yxy(sceneColor);
  Yxy.x /= (9.6 * luminance + EPSILON);
  sceneColor = Yxy2RGB(Yxy);
#else
  sceneColor *= u_PC.exposure;
#endif

#if HAS_BLOOM
  const float3 bloom = t_Bloom.Sample(s_0, input.texCoord).rgb;
  sceneColor = lerp(sceneColor, bloom, u_PC.bloomStrength);
#endif

  float3 fragColor;
  switch (u_PC.tonemap) {
  case TonemapMode::ACES:
    fragColor = ACES(sceneColor);
    break;
  case TonemapMode::Filmic:
    fragColor = tonemapFilmic(sceneColor);
    break;
  case TonemapMode::Reinhard:
    fragColor = reinhard2(sceneColor, 4.0);
    break;
  case TonemapMode::Uncharted:
    fragColor = tonemapUncharted2(sceneColor);
    break;
  case TonemapMode::Clamp:
  default:
    fragColor = sceneColor;
    break;
  }
  return float4(linearTosRGB(fragColor), 1.0);
}
