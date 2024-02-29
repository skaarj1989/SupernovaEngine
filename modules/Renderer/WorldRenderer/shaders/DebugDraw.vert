#version 460 core

layout(location = 0) in vec4 a_Position; // in world-space (.w = point size)
layout(location = 1) in vec4 a_Color;

#include "Resources/CameraBlock.glsl"

struct Instance {   // ArrayStride = 80
  mat4 modelMatrix; // offset = 0 | size = 64
  vec4 color;       //         64 |        16
};
#include "Resources/InstanceBuffer.glsl"

layout(location = 0) out vec4 v_Color;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

void main() {
  v_Color = a_Color;

#if TRIANGLE_MESH
  const Instance instance = GET_INSTANCE();
  v_Color *= instance.color;
  gl_Position =
    u_Camera.viewProjection * instance.modelMatrix * vec4(a_Position.xyz, 1.0);
#else
  gl_Position = u_Camera.viewProjection * vec4(a_Position.xyz, 1.0);
  gl_PointSize = a_Position.w;
#endif
}
