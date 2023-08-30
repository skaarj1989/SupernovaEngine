#version 460 core

layout(points) in;

in gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
}
gl_in[];

layout(location = 0) in VertexData { flat ivec3 cellIndex; }
gs_in[];

layout(points, max_vertices = 1) out;
layout(location = 0) out FragData { flat ivec3 cellIndex; }
gs_out;

void main() {
  gl_Position = gl_in[0].gl_Position;
  gl_PointSize = gl_in[0].gl_PointSize;
  gl_Layer = gs_in[0].cellIndex.z;

  gs_out.cellIndex = gs_in[0].cellIndex;

  EmitVertex();
  EndPrimitive();
}
