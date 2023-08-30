#version 460 core

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec2 v_TexCoord;

#include "FullScreenTriangle.glsl"

void main() { v_TexCoord = emitVertex(); }
