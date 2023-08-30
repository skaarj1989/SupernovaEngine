#version 460 core

layout(location = 0) in vec4 a_Position; // in world-space (.w = point size)
layout(location = 1) in vec4 a_Color;

layout(push_constant) uniform _PushConstants { mat4 u_ViewProjMatrix; };

layout(location = 0) out vec4 v_Color;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
};

void main() {
  v_Color = a_Color;
  
  gl_Position = u_ViewProjMatrix * vec4(a_Position.xyz, 1.0);
  gl_PointSize = a_Position.w;
}
