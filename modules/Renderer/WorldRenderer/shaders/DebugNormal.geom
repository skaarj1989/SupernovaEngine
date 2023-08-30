#version 460 core
#extension GL_EXT_control_flow_attributes : require

// Use with Mesh.vert

// Position and Normal in view-space

layout(triangles) in;
in gl_PerVertex { vec4 gl_Position; }
gl_in[];

#include "VertexData.glsl"

vec3 getVertexNormal(uint index) {
#ifdef HAS_NORMAL
  return normalize(
#  ifdef HAS_TANGENTS
    gs_in[index].TBN[2]
#  else
    gs_in[index].normal
#  endif
  );
#else
  return vec3(0.0);
#endif
}

#include "Resources/CameraBlock.glsl"

void emitLine(vec4 P, vec4 offset) {
  gl_Position = u_Camera.projection * P;
  EmitVertex();

  gl_Position = u_Camera.projection * (P + offset);
  EmitVertex();
  EndPrimitive();
}

#include "Lib/Math.glsl"
#include "Lib/DecomposeMatrix.glsl"

layout(line_strip, max_vertices = 6) out;

void main() {
  const float kMagnitude = 0.05;
  const float scale = max3(decomposeScale(gs_in[0].modelMatrix)) * kMagnitude;
  [[unroll]] for (uint i = 0; i < 3; ++i) {
    emitLine(gl_in[i].gl_Position, vec4(getVertexNormal(i), 0.0) * scale);
  }
}
