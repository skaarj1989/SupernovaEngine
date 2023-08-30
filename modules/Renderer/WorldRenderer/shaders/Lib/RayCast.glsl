#ifndef _RAYCAST_GLSL_
#define _RAYCAST_GLSL_

struct RayCastResult {
  bool hit;
  vec2 uv;
};
RayCastResult rayCast(vec3 rayOrigin, vec3 rayDir, float stepLength,
                      int maxSteps, int maxHits) {
  const vec3 stepInc = rayDir * stepLength;
  vec3 currentPos = rayOrigin;

  uint hitCount = 0;
  for (uint i = 0; i < maxSteps; ++i) {
    currentPos += stepInc;

    const vec2 texCoord = viewToNDC(vec4(currentPos, 1.0)).xy * 0.5 + 0.5;
    if (any(lessThan(texCoord, vec2(0.0))) ||
        any(greaterThan(texCoord, vec2(1.0)))) {
      break;
    }
    const float depth = getDepth(sampler2D(t_SceneDepth, s_0), texCoord);
    if (depth >= 1.0) continue;

    const vec3 samplePos = viewPositionFromDepth(depth, texCoord);
    const float deltaDepth = currentPos.z - samplePos.z;
    if (deltaDepth <= 0.0 && abs(rayDir.z - deltaDepth) < 1.0) {
      ++hitCount;
    }

    if (hitCount > maxHits) return RayCastResult(true, texCoord);
  }
  return RayCastResult(false, vec2(0.0));
}

#endif
