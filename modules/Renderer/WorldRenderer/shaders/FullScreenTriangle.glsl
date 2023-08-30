#ifndef _FULL_SCREEN_TRIANGLE_GLSL_
#define _FULL_SCREEN_TRIANGLE_GLSL_

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/

// vkCmdDraw(cb, 3, 1, 0, 0);
vec2 emitVertex() {
  // Vertices in clock-wise order (texCoord origin in upper left)
  const vec2 texCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
  gl_Position = vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);
  return texCoord;
}

#endif
