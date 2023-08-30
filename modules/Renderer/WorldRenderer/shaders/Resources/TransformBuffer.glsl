#ifndef _TRANSFORM_BUFFER_GLSL_
#define _TRANSFORM_BUFFER_GLSL_

layout(set = 1, binding = 1, std430) buffer readonly _TransformBuffer {
  mat4 g_Transforms[]; // ArrayStride = 64
};

mat3 _buildNormalMatrix(mat4 m) { return transpose(inverse(mat3(m))); }

#endif
