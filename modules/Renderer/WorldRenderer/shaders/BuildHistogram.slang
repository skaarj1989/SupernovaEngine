#include "Resources/Histogram.h.slang"

#include "Lib/Math.h.slang"
#include "Lib/Color.h.slang"

struct PushConstants {
  float minLogLuminance;
  float oneOverLogLuminanceRange;
  uint2 extent;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(0, 0)]]
Texture2D t_SceneColor;

groupshared uint histogram[NUM_HISTOGRAM_BINS];

uint colorToBin(float3 hdrColor) {
  const float luminance = getLuminance(hdrColor);
  if (luminance < EPSILON) return 0;

  const float logLuminance = clamp01((log2(luminance) - u_PC.minLogLuminance) *
                                     u_PC.oneOverLogLuminanceRange);
  return uint(logLuminance * 254.0 + 1.0);
}

[shader("compute") numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID,
          uint3 groupThreadID: SV_GroupThreadID,
          uint groupIndex: SV_GroupIndex) {
  histogram[groupIndex] = 0;

  GroupMemoryBarrierWithGroupSync();

  if (all(dispatchThreadID.xy < u_PC.extent)) {
    const float3 hdrColor = t_SceneColor.Load(int3(dispatchThreadID.xy, 0)).rgb;
    const uint binIndex = colorToBin(hdrColor);
    InterlockedAdd(histogram[binIndex], 1);
  }

  GroupMemoryBarrierWithGroupSync();

  InterlockedAdd(g_Histogram[groupIndex], histogram[groupIndex]);
}
