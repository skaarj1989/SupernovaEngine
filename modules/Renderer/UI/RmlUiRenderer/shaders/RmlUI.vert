#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;

layout(push_constant) uniform _PC {
  mat4 transform;
  vec2 translate;
}
u_PC;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out VertexData {
  vec2 texCoord;
  vec4 color;
}
vs_out;

void main() {
  vs_out.texCoord = a_TexCoord;
  vs_out.color = a_Color;
  gl_Position = u_PC.transform * vec4(a_Position + u_PC.translate.xy, 0.0, 1.0);
}
