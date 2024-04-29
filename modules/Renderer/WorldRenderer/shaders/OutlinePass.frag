#version 460 core

// https://www.shadertoy.com/view/sltcRf

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"

#include "Lib/Math.glsl"

layout(set = 2, binding = 0) uniform sampler2D t_SilhouetteDepth;
layout(set = 2, binding = 1) uniform usampler2D t_SilhouetteStencil;

layout(set = 2, binding = 2) uniform sampler2D t_SceneDepth;

layout(push_constant) uniform _PushConstants {
  vec4 color;
  uint numSteps;
  float radius;
}
u_PC;

layout(location = 0) out vec4 FragColor;
void main() {
  const vec2 aspect = 1.0 / getResolution();

  float outline = 0.0;
  float occlusion = 1.0;
  for (float i = 0.0; i < TAU; i += TAU / u_PC.numSteps) {
    const vec2 offset = vec2(sin(i), cos(i)) * aspect * u_PC.radius;
    const vec2 texCoord = v_TexCoord + offset;

    const float a = texture(t_SilhouetteStencil, texCoord).r;
    outline = mix(outline, 1.0, a);

    if (texture(t_SilhouetteDepth, texCoord).r -
          texture(t_SceneDepth, texCoord).r <
        EPSILON) {
      occlusion = 0.0;
    }
  }
  outline =
    clamp01(mix(outline, 0.0, texture(t_SilhouetteStencil, v_TexCoord).r));

  const vec4 kOccludedColor = vec4(0.5, 0.5, 0.5, 0.8);
  FragColor = mix(u_PC.color, kOccludedColor, occlusion) * outline;
}
