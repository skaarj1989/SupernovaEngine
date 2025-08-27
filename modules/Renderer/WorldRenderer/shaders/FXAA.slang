//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

// https://www.shadertoy.com/view/ls3GWS
// https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

#include "Resources/CameraBlock.h.slang"
#include "Lib/Color.h.slang"

[[vk::binding(0, 2)]]
Sampler2D t_0;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

[shader("fragment")]
float3 main(VSOutput input) : SV_Target {
  const float2 rcpFrame = u_Camera.getScreenTexelSize();
  const float2 uv2 = input.texCoord;
  const float4 uv = float4(uv2, uv2 - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

  const float3 rgbNW = t_0.SampleLevel(uv.zw, 0.0).xyz;
  const float3 rgbNE =
    t_0.SampleLevel(uv.zw + float2(1.0, 0.0) * rcpFrame.xy, 0.0).xyz;
  const float3 rgbSW =
    t_0.SampleLevel(uv.zw + float2(0.0, 1.0) * rcpFrame.xy, 0.0).xyz;
  const float3 rgbSE =
    t_0.SampleLevel(uv.zw + float2(1.0, 1.0) * rcpFrame.xy, 0.0).xyz;
  const float3 rgbM = t_0.SampleLevel(uv.xy, 0.0).xyz;

  const float lumaNW = getLuminance(rgbNW);
  const float lumaNE = getLuminance(rgbNE);
  const float lumaSW = getLuminance(rgbSW);
  const float lumaSE = getLuminance(rgbSE);
  const float lumaM = getLuminance(rgbM);

  const float lumaMin =
    min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  const float lumaMax =
    max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  float2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

  const float dirReduce =
    max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
  const float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

  dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) *
        rcpFrame.xy;

  const float3 rgbA =
    (1.0 / 2.0) * (t_0.SampleLevel(uv.xy + dir * (1.0 / 3.0 - 0.5), 0.0).xyz +
                   t_0.SampleLevel(uv.xy + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
  const float3 rgbB =
    rgbA * (1.0 / 2.0) +
    (1.0 / 4.0) * (t_0.SampleLevel(uv.xy + dir * (0.0 / 3.0 - 0.5), 0.0).xyz +
                   t_0.SampleLevel(uv.xy + dir * (3.0 / 3.0 - 0.5), 0.0).xyz);

  const float lumaB = getLuminance(rgbB);
  return (lumaB < lumaMin) || (lumaB > lumaMax) ? rgbA : rgbB;
}
