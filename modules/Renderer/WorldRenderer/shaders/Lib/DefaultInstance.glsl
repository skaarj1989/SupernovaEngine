#ifndef _DEFAULT_INSTANCE_GLSL_
#define _DEFAULT_INSTANCE_GLSL_

// UploadInstances.cpp

struct Instance {   // ArrayStride = 16
  uint transformId; // offset = 0 | size = 4
  uint skinOffset;  //          4 |        4
  uint materialId;  //          8 |        4
  uint flags;       //         12 |        4
};

#endif
