//
// Compute Shader:
//

#include <Lib/ImportanceSampling.h.slang>

struct PushConstants {
  uint distribution;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

[[vk::binding(0, 0), vk::image_format("rg16f")]]
RWTexture2D<float4> i_Output;

float3 integrateBRDF(float NdotV, float roughness) {
  // Compute spherical view vector: (sin(phi), 0, cos(phi))
  const float3 V = float3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
  // The macro surface normal just points up.
  const float3 N = float3(0.0, 0.0, 1.0);

  // To make the LUT independant from the material's F0, which is part of the
  // Fresnel term when substituted by Schlick's approximation, we factor it out
  // of the integral, yielding to the form: F0 * I1 + I2 I1 and I2 are slighlty
  // different in the Fresnel term, but both only depend on NoL and roughness,
  // so they are both numerically integrated and written into two channels.
  float A = 0.0;
  float B = 0.0;
  float C = 0.0;

  const uint kNumSamples = 1024;
  for (uint i = 0; i < kNumSamples; ++i) {
    // Importance sampling, depending on the distribution.
    const float4 importanceSample =
      getImportanceSample(u_PC.distribution, i, kNumSamples, N, roughness);
    const float3 H = importanceSample.xyz;
    const float3 L = normalize(reflect(-V, H));

    const float NdotL = clamp01(L.z);
    const float NdotH = clamp01(H.z);
    const float VdotH = clamp01(dot(V, H));
    if (NdotL > 0.0) {
      if (u_PC.distribution == DISTRIBUTION_GGX) {
        // LUT for GGX distribution.

        // Taken from: https://bruop.github.io/ibl
        // Shadertoy: https://www.shadertoy.com/view/3lXXDB
        // Terms besides V are from the GGX PDF we're dividing by.
        const float V_pdf =
          V_SmithGGXCorrelated(NdotV, NdotL, roughness) * VdotH * NdotL / NdotH;
        const float Fc = pow(1.0 - VdotH, 5.0);
        A += (1.0 - Fc) * V_pdf;
        B += Fc * V_pdf;
      } else if (u_PC.distribution == DISTRIBUTION_CHARLIE) {
        // LUT for Charlie distribution.
        const float sheenDistribution = D_Charlie(roughness, NdotH);
        const float sheenVisibility = V_Ashikhmin(NdotL, NdotV);
        C += sheenVisibility * sheenDistribution * NdotL * VdotH;
      }
    }
  }

  // The PDF is simply pdf(v, h) -> NDF * <nh>.
  // To parametrize the PDF over l, use the Jacobian transform, yielding to:
  // pdf(v, l) -> NDF * <nh> / 4<vh> Since the BRDF divide through the PDF to be
  // normalized, the 4 can be pulled out of the integral.
  return float3(4.0 * A, 4.0 * B, 4.0 * 2.0 * PI * C) / float(kNumSamples);
}

[shader("compute") numthreads(32, 32, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
  uint2 imageSize;
  i_Output.GetDimensions(imageSize.x, imageSize.y);
  const float2 texelSize = 1.0 / float2(imageSize);
  const int2 coord = int2(dispatchThreadID.xy);

  const float2 texCoord = float2(coord + 1) * texelSize;
  const float NdotV = texCoord.x;
  const float roughness = texCoord.y;

  i_Output[coord] = float4(integrateBRDF(NdotV, roughness), 0.0);
}
