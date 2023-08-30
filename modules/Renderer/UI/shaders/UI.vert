#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;

#if USE_PROJECTION_MATRIX
layout(push_constant) uniform _PC { mat4 u_ProjectionMatrix; };
#else
layout(push_constant) uniform _PC {
  vec2 scale;
  vec2 translate;
}
u_PC;
#endif

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out VertexData {
  vec2 texCoord;
  vec4 color;
}
vs_out;

void main() {
  vs_out.texCoord = a_TexCoord;
  vs_out.color = a_Color;
#if USE_PROJECTION_MATRIX
  gl_Position = u_ProjectionMatrix * vec4(a_Position, 0.0, 1.0);
#else
  gl_Position = vec4(a_Position * u_PC.scale + u_PC.translate, 0.0, 1.0);
#endif
}
