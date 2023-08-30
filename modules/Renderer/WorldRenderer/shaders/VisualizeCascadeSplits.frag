#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"
#include "Resources/ShadowBlock.glsl"

layout(set = 2, binding = 1) uniform texture2D t_SceneDepth;

#include "Lib/Depth.glsl"

layout(location = 0) out vec4 FragColor;
void main() {
  const float depth = fetchDepth(t_SceneDepth, ivec2(gl_FragCoord.xy));
  if (depth >= 1.0) discard; // Don't touch the far plane

  const vec3 fragPosViewSpace = viewPositionFromDepth(depth, v_TexCoord);
  switch (_selectCascadeIndex(fragPosViewSpace)) {
  case 0:
    FragColor.rgb = vec3(1.00, 0.25, 0.25);
    break;
  case 1:
    FragColor.rgb = vec3(0.25, 1.00, 0.25);
    break;
  case 2:
    FragColor.rgb = vec3(0.25, 0.25, 1.00);
    break;
  case 3:
    FragColor.rgb = vec3(1.00, 1.00, 0.25);
    break;
  }
  FragColor.a = 0.35; // Overlay intensity
}
