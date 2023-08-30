#ifndef _SKIN_BUFFER_GLSL_
#define _SKIN_BUFFER_GLSL_

layout(set = 0, binding = 2, std430) buffer readonly _SkinBuffer {
  mat4 g_Skins[]; // ArrayStride = 64
};

#ifdef IS_SKINNED
mat4 _buildSkinMatrix(uint offset) {
  return (a_Weights[0] * g_Skins[offset + a_Joints[0]] +
          a_Weights[1] * g_Skins[offset + a_Joints[1]] +
          a_Weights[2] * g_Skins[offset + a_Joints[2]] +
          a_Weights[3] * g_Skins[offset + a_Joints[3]]);
}
#endif

#endif
