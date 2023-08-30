#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler s_0;

layout(set = 0, binding = 1) uniform texture2D t_SceneColor;
#if HAS_EYE_ADAPTATION
layout(set = 0, binding = 2, r16f) uniform readonly image2D i_AverageLuminance;
#endif
#if HAS_BLOOM
layout(set = 0, binding = 3) uniform texture2D t_Bloom;
#endif

#include "Tonemapping/ACES.glsl"
#include "Tonemapping/Filmic.glsl"
#include "Tonemapping/Reinhard.glsl"
#include "Tonemapping/Uncharted.glsl"

const uint Tonemap_Clamp = 0;
const uint Tonemap_ACES = 1;
const uint Tonemap_Filmic = 2;
const uint Tonemap_Reinhard = 3;
const uint Tonemap_Uncharted = 4;

layout(push_constant) uniform _PushConstants {
  uint tonemap;
  float exposure;
  float bloomStrength;
}
u_PC;

#include "Lib/Math.glsl"
#include "Lib/Color.glsl"

layout(location = 0) out vec4 FragColor;
void main() {
  vec3 sceneColor = texture(sampler2D(t_SceneColor, s_0), v_TexCoord).rgb;

#if HAS_EYE_ADAPTATION
  float luminance = imageLoad(i_AverageLuminance, ivec2(0)).r;
  vec3 Yxy = RGB2Yxy(sceneColor);
  Yxy.x /= (9.6 * luminance + EPSILON);
  sceneColor = Yxy2RGB(Yxy);
#else
  sceneColor *= u_PC.exposure;
#endif

#if HAS_BLOOM
  const vec3 bloom = texture(sampler2D(t_Bloom, s_0), v_TexCoord).rgb;
  sceneColor = mix(sceneColor, bloom, u_PC.bloomStrength);
#endif

  switch (u_PC.tonemap) {
  case Tonemap_ACES:
    FragColor.rgb = ACES(sceneColor);
    break;
  case Tonemap_Filmic:
    FragColor.rgb = tonemapFilmic(sceneColor);
    break;
  case Tonemap_Reinhard:
    FragColor.rgb = reinhard2(sceneColor, 4.0);
    break;
  case Tonemap_Uncharted:
    FragColor.rgb = tonemapUncharted2(sceneColor);
    break;
  case Tonemap_Clamp:
  default:
    FragColor.rgb = sceneColor;
    break;
  }
  FragColor = vec4(linearTosRGB(FragColor.rgb), 1.0);
}
