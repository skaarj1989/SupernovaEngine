#version 460 core

// Use with Mesh.vert

layout(push_constant) uniform _PushConstants {
  // skip u_InstanceOffset (vertex shader)
  layout(offset = 16) vec4 u_Color;
};

layout(location = 0) out vec4 FragColor;
void main() { FragColor = u_Color; }
