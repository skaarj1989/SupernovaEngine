#version 460 core

#include "Resources/SceneGridBlock.glsl"
#include "Lib/Math.glsl"

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

layout(location = 0) out VertexData { flat ivec3 cellIndex; }
vs_out;

void main() {
  const vec3 position =
    vec3(unflatten3D(gl_VertexIndex, u_SceneGrid.gridSize.xy));

  vs_out.cellIndex = ivec3(position);

  const vec2 ndc =
    (position.xy + 0.5) / vec2(u_SceneGrid.gridSize.xy) * 2.0 - 1.0;
  gl_Position = vec4(ndc, 0.0, 1.0);

  gl_PointSize = 1.0;
}
