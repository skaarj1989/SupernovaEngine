#ifndef _SHADOW_BLOCK_GLSL_
#define _SHADOW_BLOCK_GLSL_

#define MAX_NUM_CASCADES 4
#define MAX_NUM_SPOTLIGHT_SHADOWS 4

// clang-format off
struct Cascades {                          // ArrayStride = 272
  vec4 splitDepth;                         // offset = 0 | size = 16
  mat4 viewProjMatrices[MAX_NUM_CASCADES]; //         16 |       256 (4 cascades)
};

layout(set = 1, binding = 4, std140) uniform _ShadowBlock {
  Cascades cascades;                                  // offset = 0 | size = 272
  mat4 spotLightsViewProj[MAX_NUM_SPOTLIGHT_SHADOWS]; //        272 |        256
}
u_ShadowBlock;
// clang-format on

uint _selectCascadeIndex(vec3 fragPosViewSpace) {
  uint cascadeIndex = 0;
  for (uint i = 0; i < MAX_NUM_CASCADES - 1; ++i) {
    const float splitDepth = u_ShadowBlock.cascades.splitDepth[i];
    if (fragPosViewSpace.z < splitDepth) cascadeIndex = i + 1;
  }
  return cascadeIndex;
}

#endif
