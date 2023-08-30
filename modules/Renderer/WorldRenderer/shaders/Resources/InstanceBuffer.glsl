#ifndef _INSTANCE_BUFFER_GLSL_
#define _INSTANCE_BUFFER_GLSL_

#ifndef GL_VERTEX_SHADER
#  error "VertexShader only!"
#endif

// UploadInstances.cpp

struct Instance {   // ArrayStride = 16
  uint transformId; // offset = 0 | size = 4
  uint skinOffset;  //          4 |        4
  uint materialId;  //          8 |        4
  uint flags;       //         12 |        4
};

layout(set = 1, binding = 2, std430) buffer readonly _InstanceBuffer {
  Instance g_Instances[];
};

layout(push_constant) uniform _PushConstants { uint u_InstanceOffset; };

#define GET_INSTANCE() g_Instances[u_InstanceOffset + gl_InstanceIndex]

#endif
