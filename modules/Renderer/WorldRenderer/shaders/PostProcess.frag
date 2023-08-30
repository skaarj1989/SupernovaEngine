#version 460 core

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Lib/Depth.glsl"

layout(set = 2, binding = 0) uniform sampler2D t_SceneDepth;
layout(set = 2, binding = 1) uniform sampler2D t_SceneColor;

vec3 g_fragPosWorldSpace = vec3(0.0);

vec3 getPosition() { return g_fragPosWorldSpace; }
vec2 getTexCoord0() { return v_TexCoord; }

#if HAS_PROPERTIES
#  include "Resources/PropertyBuffer.glsl"
_DECLARE_PROPERTIES(0, 1);
#endif

#include "UserCodeCommons.glsl"

#include "FragmentPosition.glsl"

#region USER_SAMPLERS

#region USER_MODULES

// clang-format off
void _executeUserCode(
  FragmentPosition fragPos,
#if HAS_PROPERTIES
  in Properties properties,
#endif
  out vec4 fragColor) {
#region USER_CODE
} // clang-format on

layout(location = 0) out vec4 FragColor;
void main() {
  const FragmentPosition fragPos =
    buildFragPosFromDepthSampler(t_SceneDepth, v_TexCoord);
  g_fragPosWorldSpace = fragPos.worldSpace;

  // clang-format off
  _executeUserCode(
    fragPos,
#if HAS_PROPERTIES
    u_Properties[0],
#endif
    FragColor
  );
  // clang-format on
}
