#version 460 core
#extension GL_EXT_control_flow_attributes : require

layout(location = 0) in vec2 v_TexCoord;

layout(set = 0, binding = 0) uniform sampler s_0;
layout(set = 0, binding = 1) uniform texture2D t_0[NUM_TEXTURES];

layout(location = 0) out vec4 FragColor;
void main() {
  vec4 color = vec4(0.0);
  [[unroll]] for (int i = 0; i < NUM_TEXTURES; ++i) {
    color += texture(sampler2D(t_0[i], s_0), v_TexCoord);
  }
  FragColor = color;
}
