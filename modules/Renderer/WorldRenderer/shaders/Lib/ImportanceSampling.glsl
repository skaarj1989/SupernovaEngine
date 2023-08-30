#ifndef _IMPORTANCE_SAMPLE_GLSL_
#define _IMPORTANCE_SAMPLE_GLSL_

// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/ba6678f35ce33d7f48c00dd070435207efafaa46/source/shaders/ibl_filtering.frag

#include "Math.glsl"
#include "Hammersley2D.glsl"

#define DISTRIBUTION_LAMBERTIAN 0
#define DISTRIBUTION_GGX 1
#define DISTRIBUTION_CHARLIE 2

// @param N must be normalized
// @return TBN coordinate frame from the normal
mat3 generateTBN(vec3 N) {
  vec3 B = vec3(0.0, 1.0, 0.0);
  const float NdotUp = dot(N, vec3(0.0, 1.0, 0.0));
  if (1.0 - abs(NdotUp) <= EPSILON) {
    // Sampling +Y or -Y, so we need a more robust bitangent.
    if (NdotUp > 0.0) {
      B = vec3(0.0, 0.0, 1.0);
    } else {
      B = vec3(0.0, 0.0, -1.0);
    }
  }
  const vec3 T = normalize(cross(B, N));
  B = cross(N, T);
  return mat3(T, B, N);
}

float D_GGX(float NdotH, float roughness) {
  const float a = NdotH * roughness;
  const float k = roughness / (1.0 - NdotH * NdotH + a * a);
  return k * k * (1.0 / PI);
}

// NDF
float D_Ashikhmin(float NdotH, float roughness) {
  const float alpha = roughness * roughness;
  // Ashikhmin 2007, "Distribution-based BRDFs"
  const float a2 = alpha * alpha;
  const float cos2h = NdotH * NdotH;
  const float sin2h = 1.0 - cos2h;
  const float sin4h = sin2h * sin2h;
  const float cot2 = -cos2h / (a2 * sin2h);
  return 1.0 / (PI * (4.0 * a2 + 1.0) * sin4h) * (4.0 * exp(cot2) + sin4h);
}

// NDF
float D_Charlie(float sheenRoughness, float NdotH) {
  sheenRoughness = max(sheenRoughness, EPSILON); // clamp (0,1]
  const float invR = 1.0 / sheenRoughness;
  const float cos2h = NdotH * NdotH;
  const float sin2h = 1.0 - cos2h;
  return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

// From the filament docs. Geometric Shadowing function
// https://google.github.io/filament/Filament.html#toc4.4.2
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
  const float a2 = pow(roughness, 4.0);
  const float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
  const float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
  return 0.5 / (GGXV + GGXL);
}

// https://github.com/google/filament/blob/master/shaders/src/brdf.fs#L136
float V_Ashikhmin(float NdotL, float NdotV) {
  return clamp01(1.0 / (4.0 * (NdotL + NdotV - NdotL * NdotV)));
}

struct MicrofacetDistributionSample {
  float pdf;
  float cosTheta;
  float sinTheta;
  float phi;
};

MicrofacetDistributionSample Lambertian(vec2 xi, float roughness) {
  MicrofacetDistributionSample lambertian;

  // Cosine weighted hemisphere sampling
  // http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations.html#Cosine-WeightedHemisphereSampling
  lambertian.cosTheta = sqrt(1.0 - xi.y);
  // equivalent to `sqrt(1.0 - cosTheta*cosTheta)`;
  lambertian.sinTheta = sqrt(xi.y);
  lambertian.phi = 2.0 * PI * xi.x;

  // evaluation for solid angle, therefore drop the sinTheta
  lambertian.pdf = lambertian.cosTheta / PI;

  return lambertian;
}

// GGX microfacet distribution
// https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
// This implementation is based on https://bruop.github.io/ibl/,
//  https://www.tobias-franke.eu/log/2014/03/30/notes_on_importance_sampling.html
// and https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html
MicrofacetDistributionSample GGX(vec2 xi, float roughness) {
  MicrofacetDistributionSample ggx;

  // evaluate sampling equations
  const float alpha = roughness * roughness;
  ggx.cosTheta =
    clamp01(sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y)));
  ggx.sinTheta = sqrt(1.0 - ggx.cosTheta * ggx.cosTheta);
  ggx.phi = 2.0 * PI * xi.x;

  // evaluate GGX pdf (for half vector)
  ggx.pdf = D_GGX(ggx.cosTheta, alpha);

  // Apply the Jacobian to obtain a pdf that is parameterized by l
  // see https://bruop.github.io/ibl/
  // Typically you'd have the following:
  // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
  // but since V = N => VoH == NoH
  ggx.pdf /= 4.0;

  return ggx;
}

MicrofacetDistributionSample Charlie(vec2 xi, float roughness) {
  MicrofacetDistributionSample charlie;

  const float alpha = roughness * roughness;
  charlie.sinTheta = pow(xi.y, alpha / (2.0 * alpha + 1.0));
  charlie.cosTheta = sqrt(1.0 - charlie.sinTheta * charlie.sinTheta);
  charlie.phi = 2.0 * PI * xi.x;

  // evaluate Charlie pdf (for half vector)
  charlie.pdf = D_Charlie(alpha, charlie.cosTheta);

  // Apply the Jacobian to obtain a pdf that is parameterized by l
  charlie.pdf /= 4.0;

  return charlie;
}

float computeLod(float pdf, uint width, uint sampleCount) {
  // // Solid angle of current sample -- bigger for less likely samples
  // float omegaS = 1.0 / (float(sampleCount) * pdf);
  // // Solid angle of texel
  // // note: the factor of 4.0 * PI
  // float omegaP = 4.0 * PI / (6.0 * float(width) * float(width));
  // // Mip level is determined by the ratio of our sample's solid angle to a
  // texel's solid angle
  // // note that 0.5 * log2 is equivalent to log4
  // float lod = 0.5 * log2(omegaS / omegaP);

  // babylon introduces a factor of K (=4) to the solid angle ratio
  // this helps to avoid undersampling the environment map
  // this does not appear in the original formulation by Jaroslav Krivanek and
  // Mark Colbert log4(4) == 1 lod += 1.0;

  // We achieved good results by using the original formulation from Krivanek &
  // Colbert adapted to cubemaps

  // https://cgg.mff.cuni.cz/~jaroslav/papers/2007-sketch-fis/Final_sap_0073.pdf
  return 0.5 *
         log2(6.0 * float(width) * float(width) / (float(sampleCount) * pdf));
}

// @return importance sample direction with pdf in the .w component
vec4 getImportanceSample(uint distribution, uint sampleIndex, uint sampleCount,
                         vec3 N, float roughness) {
  // generate a quasi monte carlo point in the unit square [0.1)^2
  const vec2 xi = hammersley2D(sampleIndex, sampleCount);

  MicrofacetDistributionSample importanceSample;

  // generate the points on the hemisphere with a fitting mapping for
  // the distribution (e.g. lambertian uses a cosine importance)
  if (distribution == DISTRIBUTION_LAMBERTIAN) {
    importanceSample = Lambertian(xi, roughness);
  } else if (distribution == DISTRIBUTION_GGX) {
    // Trowbridge-Reitz / GGX microfacet model (Walter et al)
    // https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
    importanceSample = GGX(xi, roughness);
  } else if (distribution == DISTRIBUTION_CHARLIE) {
    importanceSample = Charlie(xi, roughness);
  }

  // transform the hemisphere sample to the normal coordinate frame
  // i.e. rotate the hemisphere to the normal direction
  const vec3 localSpaceDirection =
    normalize(vec3(importanceSample.sinTheta * cos(importanceSample.phi),
                   importanceSample.sinTheta * sin(importanceSample.phi),
                   importanceSample.cosTheta));
  const mat3 TBN = generateTBN(N);
  const vec3 direction = TBN * localSpaceDirection;

  return vec4(direction, importanceSample.pdf);
}

#endif
