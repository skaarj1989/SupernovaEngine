#version 460 core

layout(location = 0) in vec4 a_Position; // in world-space (.w = point size)
layout(location = 1) in vec4 a_Color;

#if TRIANGLE_MESH
struct Instance {
  mat4 modelMatrix;
  vec4 color;
};
layout(set = 0, binding = 0, std430) buffer readonly _InstanceBuffer {
  Instance g_Instances[];
};
#endif

layout(push_constant) uniform _PushConstants {
  mat4 u_ViewProjMatrix;
#if TRIANGLE_MESH
  uint u_InstanceOffset;
#endif
};

layout(location = 0) out vec4 v_Color;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

void main() {
  v_Color = a_Color;

#if TRIANGLE_MESH
  const Instance instance = g_Instances[u_InstanceOffset + gl_InstanceIndex];
  v_Color *= instance.color;
  gl_Position =
    u_ViewProjMatrix * instance.modelMatrix * vec4(a_Position.xyz, 1.0);
#else
  gl_Position = u_ViewProjMatrix * vec4(a_Position.xyz, 1.0);
  gl_PointSize = a_Position.w;
#endif
}
