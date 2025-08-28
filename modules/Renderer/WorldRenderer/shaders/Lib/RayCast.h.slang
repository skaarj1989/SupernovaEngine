#pragma once

#include "SpaceTraversal.h.slang"
#include "Depth.h.slang"

struct RayCastResult {
  bool hit;
  float2 uv;
};
RayCastResult rayCast(float3 rayOrigin, float3 rayDir, float stepLength,
                      int maxSteps, int maxHits) {
  const float3 stepInc = rayDir * stepLength;
  float3 currentPos = rayOrigin;

  uint hitCount = 0;
  for (uint i = 0; i < maxSteps; ++i) {
    currentPos += stepInc;

    const float2 texCoord = viewToNDC(float4(currentPos, 1.0)).xy * 0.5 + 0.5;
    if (any(texCoord < float2(0.0)) || any(texCoord > float2(1.0))) {
      break;
    }
    const float depth = getDepth(t_SceneDepth, s_0, texCoord);
    if (depth >= 1.0) continue;

    const float3 samplePos = viewPositionFromDepth(depth, texCoord);
    const float deltaDepth = currentPos.z - samplePos.z;
    if (deltaDepth <= 0.0 && abs(rayDir.z - deltaDepth) < 1.0) {
      ++hitCount;
    }

    if (hitCount > maxHits) return RayCastResult(true, texCoord);
  }
  return RayCastResult(false, float2(0.0));
}
