#ifndef _IDS_GLSL
#define _IDS_GLSL

#ifndef GL_FRAGMENT_SHADER
#  error "FragmentShader only!"
#endif

#include <Resources/CameraBlock.glsl>
#include <Lib/Math.glsl>

#if WRITE_USERDATA
layout(set = 2, binding = 13, std430) buffer _ID {
  uint g_IDs[]; // ArrayStride = 4
};

void writeUserData(uint id) {
  const uvec2 coords = uvec2(gl_FragCoord.xy);
  g_IDs[flatten2D(coords, u_Camera.resolution.x)] = id;
}
#endif

#endif
