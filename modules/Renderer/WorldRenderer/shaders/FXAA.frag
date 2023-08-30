#version 460 core

// https://www.shadertoy.com/view/ls3GWS
// https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

layout(location = 0) in vec2 v_TexCoord;

#include "Resources/CameraBlock.glsl"
#include "Lib/Color.glsl"

layout(set = 2, binding = 0) uniform sampler2D t_0;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

layout(location = 0) out vec3 FragColor;
void main() {
  const vec2 rcpFrame = getScreenTexelSize();
  const vec2 uv2 = v_TexCoord;
  const vec4 uv = vec4(uv2, uv2 - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

  const vec3 rgbNW = textureLod(t_0, uv.zw, 0.0).xyz;
  const vec3 rgbNE =
    textureLod(t_0, uv.zw + vec2(1.0, 0.0) * rcpFrame.xy, 0.0).xyz;
  const vec3 rgbSW =
    textureLod(t_0, uv.zw + vec2(0.0, 1.0) * rcpFrame.xy, 0.0).xyz;
  const vec3 rgbSE =
    textureLod(t_0, uv.zw + vec2(1.0, 1.0) * rcpFrame.xy, 0.0).xyz;
  const vec3 rgbM = textureLod(t_0, uv.xy, 0.0).xyz;

  const float lumaNW = getLuminance(rgbNW);
  const float lumaNE = getLuminance(rgbNE);
  const float lumaSW = getLuminance(rgbSW);
  const float lumaSE = getLuminance(rgbSE);
  const float lumaM = getLuminance(rgbM);

  const float lumaMin =
    min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  const float lumaMax =
    max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

  const float dirReduce =
    max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
  const float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

  dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) *
        rcpFrame.xy;

  const vec3 rgbA =
    (1.0 / 2.0) * (textureLod(t_0, uv.xy + dir * (1.0 / 3.0 - 0.5), 0.0).xyz +
                   textureLod(t_0, uv.xy + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
  const vec3 rgbB =
    rgbA * (1.0 / 2.0) +
    (1.0 / 4.0) * (textureLod(t_0, uv.xy + dir * (0.0 / 3.0 - 0.5), 0.0).xyz +
                   textureLod(t_0, uv.xy + dir * (3.0 / 3.0 - 0.5), 0.0).xyz);

  const float lumaB = getLuminance(rgbB);
  FragColor = (lumaB < lumaMin) || (lumaB > lumaMax) ? rgbA : rgbB;
}
