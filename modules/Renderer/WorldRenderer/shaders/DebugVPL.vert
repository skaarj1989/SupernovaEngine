#version 460 core
#extension GL_EXT_samplerless_texture_functions : require

// .rgb = world position
layout(set = 2, binding = 0) uniform texture2D t_GBuffer0;
// .rgb = normal (world-space)
layout(set = 2, binding = 1) uniform texture2D t_GBuffer1;

#include "Resources/CameraBlock.glsl"
#include "Resources/SceneGridBlock.glsl"

#include "Lib/Math.glsl"
#include "Lib/LPV.glsl"

bool inBounds(vec3 v) {
  return all(greaterThanEqual(v, vec3(0.0))) &&
         all(lessThanEqual(v, vec3(1.0)));
}

layout(push_constant) uniform _PushConstants { uint u_RSMResolution; };

layout(location = 0) out VertexData {
  vec3 cellCoord;
  vec3 N;
}
vs_out;

void main() {
  const ivec3 id = ivec3(unflatten3D(gl_VertexIndex, u_SceneGrid.gridSize.xy));

  const ivec2 coord = ivec2(unflatten2D(gl_VertexIndex, u_RSMResolution));
  const vec3 position = texelFetch(t_GBuffer0, coord.xy, 0).rgb;
  const vec3 N = texelFetch(t_GBuffer1, coord.xy, 0).rgb;

  vs_out.cellCoord = (position - u_SceneGrid.minCorner) / u_SceneGrid.cellSize /
                     vec3(u_SceneGrid.gridSize);
  vs_out.N = inBounds(vs_out.cellCoord) ? N : vec3(0.0);

  gl_Position = u_Camera.viewProjection * vec4(position + 0.5 * N, 1.0);
  gl_PointSize = 10.0;
}
