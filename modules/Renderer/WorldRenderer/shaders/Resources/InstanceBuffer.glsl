#ifndef _INSTANCE_BUFFER_GLSL_
#define _INSTANCE_BUFFER_GLSL_

#ifndef GL_VERTEX_SHADER
#  error "VertexShader only!"
#endif

layout(set = 1, binding = 2, std430) buffer readonly _InstanceBuffer {
  Instance g_Instances[];
};

layout(push_constant) uniform _PushConstants { uint u_InstanceOffset; };

#define GET_INSTANCE() g_Instances[u_InstanceOffset + gl_InstanceIndex]

#endif
