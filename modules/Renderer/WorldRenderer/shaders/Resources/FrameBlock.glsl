#ifndef _FRAME_BLOCK_GLSL_
#define _FRAME_BLOCK_GLSL_

// UploadFrameBlock.cpp

struct Frame {     // ArrayStride = 8 bytes
  float time;      // offset = 0 | size = 4
  float deltaTime; //          4 |        4
};

layout(set = 0, binding = 0, std140) uniform _FrameBlock { Frame u_Frame; };

float getTime() { return u_Frame.time; }
float getDeltaTime() { return u_Frame.deltaTime; }

#endif
