#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

// .rgb = position (world-space)
layout(set = 2, binding = 0) uniform texture2D t_GBuffer0;
// .rgb = normal (world-space)
layout(set = 2, binding = 1) uniform texture2D t_GBuffer1;
// .rgb = flux
layout(set = 2, binding = 2) uniform texture2D t_GBuffer2;

#include "Resources/SceneGridBlock.glsl"
#include "Lib/Math.glsl"
#include "Lib/LPV.glsl"

bool inBounds(ivec3 id) {
  return all(greaterThanEqual(id, ivec3(0))) &&
         all(lessThan(id, ivec3(u_SceneGrid.gridSize)));
}

layout(push_constant) uniform _PushConstants { int u_RSMResolution; };

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

layout(location = 0) out VertexData {
  vec3 N;
  vec4 flux;
  flat ivec3 cellIndex;
}
vs_out;

void main() {
  const ivec2 coord = ivec2(unflatten2D(gl_VertexIndex, u_RSMResolution));

  const vec3 position = texelFetch(t_GBuffer0, coord, 0).xyz;
  const vec3 N = texelFetch(t_GBuffer1, coord, 0).xyz;
  const vec4 flux = texelFetch(t_GBuffer2, coord, 0);

  const ivec3 gridCell =
    ivec3((position - u_SceneGrid.minCorner) / u_SceneGrid.cellSize + 0.5 * N);

  if (inBounds(gridCell)) {
    vs_out.N = N;
    vs_out.flux = flux;
    vs_out.cellIndex = gridCell;
  } else {
    vs_out.N = vec3(0.0);
  }

  const vec2 ndc =
    (vec2(gridCell.xy) + 0.5) / u_SceneGrid.gridSize.xy * 2.0 - 1.0;
  gl_Position = vec4(ndc, 0.0, 1.0);

  gl_PointSize = 1.0;
}
