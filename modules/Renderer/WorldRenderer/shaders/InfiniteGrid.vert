#version 460 core

#include "Resources/CameraBlock.glsl"
#include "Lib/SpaceTraversal.glsl"

out gl_PerVertex { vec4 gl_Position; };

#include "FullScreenTriangle.glsl"

layout(location = 0) out VertexData {
  vec3 near;
  vec3 far;
}
vs_out;

void main() {
  const vec2 texCoord = emitVertex();
  const vec2 P = texCoord * 2.0 - 1.0;
  vs_out.near = NDCToWorld(vec4(P, 0.0, 1.0));
  vs_out.far = NDCToWorld(vec4(P, 1.0, 1.0));
}
