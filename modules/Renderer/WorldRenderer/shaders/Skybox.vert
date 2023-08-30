#version 460 core

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out VertexOutput { vec3 eyeDirection; }
vs_out;

#include "Resources/CameraBlock.glsl"

#include "Lib/SpaceTraversal.glsl"
#include "FullScreenTriangle.glsl"

void main() {
  emitVertex();

  const vec3 P = NDCToView(gl_Position);
  vs_out.eyeDirection = viewToWorld(vec4(P, 0.0));

  gl_Position = gl_Position.xyww;
}
