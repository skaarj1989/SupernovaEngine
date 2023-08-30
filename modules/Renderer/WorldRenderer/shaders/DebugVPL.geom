#version 460 core

layout(points) in;
layout(location = 0) in VertexData {
  vec3 cellCoord;
  vec3 N;
}
gs_in[];

layout(points, max_vertices = 1) out;
layout(location = 0) out FragData {
  vec3 cellCoord;
  vec3 N;
}
gs_out;

void main() {
  if (length(gs_in[0].N) < 0.01) return;

  gl_Position = gl_in[0].gl_Position;
  gl_PointSize = gl_in[0].gl_PointSize;

  gs_out.cellCoord = gs_in[0].cellCoord;
  gs_out.N = gs_in[0].N;

  EmitVertex();
  EndPrimitive();
}
