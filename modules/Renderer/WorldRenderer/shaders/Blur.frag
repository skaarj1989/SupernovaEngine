#version 460 core

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 1) uniform sampler2D t_0;

#include "Lib/Texture.glsl"
#include "Lib/Blur.glsl"

layout(push_constant) uniform _PushConstants { vec2 u_Direction; };

layout(location = 0) out vec3 FragColor;
void main() {
  const vec2 resolution = textureSize(t_0, 0);
  FragColor = blur5(t_0, v_TexCoord, resolution, u_Direction).rgb;
}
