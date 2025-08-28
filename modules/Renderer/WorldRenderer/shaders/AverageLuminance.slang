#include "Resources/Histogram.h.slang"

struct PushConstants {
  float minLogLuminance;
  float logLuminanceRange;
  float timeCoeff;
  uint numPixels;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(2, 0), vk::image_format("r16f")]]
RWTexture2D<float> i_Target;

groupshared uint histogram[NUM_HISTOGRAM_BINS];

[shader("compute") numthreads(NUM_HISTOGRAM_BINS, 1, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID,
          uint groupIndex: SV_GroupIndex) {
  const uint countForThisBin = g_Histogram[groupIndex];
  histogram[groupIndex] = countForThisBin * groupIndex;

  GroupMemoryBarrierWithGroupSync();

  [[unroll]]
  for (uint binIndex = (NUM_HISTOGRAM_BINS >> 1); binIndex > 0;
       binIndex >>= 1) {
    if (groupIndex < binIndex) {
      InterlockedAdd(histogram[groupIndex], histogram[groupIndex + binIndex]);
    }

    GroupMemoryBarrierWithGroupSync();
  }

  if (groupIndex == 0) {
    float weightedLogAverage =
      (float(histogram[0]) /
       max(float(u_PC.numPixels - countForThisBin), 1.0)) -
      1.0;

    const float weightedAverageLuminance =
      exp2(weightedLogAverage / 254.0 * u_PC.logLuminanceRange +
           u_PC.minLogLuminance);
    float luminanceLastFrame = i_Target.Load(int2(0, 0)).x;

    const float adaptedLuminance =
      luminanceLastFrame +
      (weightedAverageLuminance - luminanceLastFrame) * u_PC.timeCoeff;
    i_Target.Store(int2(0, 0), adaptedLuminance);
  }
}
