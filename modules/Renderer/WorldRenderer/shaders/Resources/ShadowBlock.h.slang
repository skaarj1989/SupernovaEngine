#pragma once

#define MAX_NUM_CASCADES 4
#define MAX_NUM_SPOTLIGHT_SHADOWS 4

struct Cascades {
  float4 splitDepth;
  float4x4 viewProjMatrices[MAX_NUM_CASCADES];
};

struct ShadowBlock {
  Cascades cascades;
  float4x4 spotLightsViewProj[MAX_NUM_SPOTLIGHT_SHADOWS];
};

[[vk::binding(4, 1)]]
ConstantBuffer<ShadowBlock> u_ShadowBlock;

uint _selectCascadeIndex(float3 fragPosViewSpace) {
  uint cascadeIndex = 0;
  for (uint i = 0; i < MAX_NUM_CASCADES - 1; ++i) {
    float splitDepth = u_ShadowBlock.cascades.splitDepth[i];
    if (fragPosViewSpace.z < splitDepth) cascadeIndex = i + 1;
  }
  return cascadeIndex;
}
