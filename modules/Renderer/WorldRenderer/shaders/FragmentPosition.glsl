#ifndef _FRAGMENT_POSITION_GLSL_
#define _FRAGMENT_POSITION_GLSL_

#include "Lib/SpaceTraversal.glsl"

struct FragmentPosition {
  vec3 worldSpace;
  vec3 viewSpace;
};

FragmentPosition buildFragPosFromWorldSpace(vec4 fragPosWorldSpace) {
  const vec3 fragPosViewSpace = worldToView(fragPosWorldSpace);
  return FragmentPosition(fragPosWorldSpace.xyz, fragPosViewSpace);
}
FragmentPosition buildFragPosFromViewSpace(vec4 fragPosViewSpace) {
  const vec3 fragPosWorldSpace = viewToWorld(fragPosViewSpace);
  return FragmentPosition(fragPosWorldSpace, fragPosViewSpace.xyz);
}

#ifdef _DEPTH_GLSL_
FragmentPosition buildFragPosFromDepth(float z, vec2 texCoord) {
  const vec3 fragPosViewSpace = viewPositionFromDepth(z, texCoord);
  return buildFragPosFromViewSpace(vec4(fragPosViewSpace, 1.0));
}
FragmentPosition buildFragPosFromDepthSampler(sampler2D sceneDepth,
                                              vec2 texCoord) {
  const float z = getDepth(sceneDepth, texCoord);
  return buildFragPosFromDepth(z, texCoord);
}
FragmentPosition buildFragPosFromDepthSampler(texture2D sceneDepth, sampler s,
                                              vec2 texCoord) {
  const float z = getDepth(sampler2D(sceneDepth, s), texCoord);
  return buildFragPosFromDepth(z, texCoord);
}
#endif

#endif
