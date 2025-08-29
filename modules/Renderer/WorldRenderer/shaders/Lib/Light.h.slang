#pragma once

enum LightType : uint {
  Directional = 0,
  Spot = 1,
  Point = 2,
}

// UploadLights.cpp
// clang-format off
struct Light {
  float4 position;  // [point/spot] .xyz = position, .w = range
  float4 direction; // [spot/directional] .xyz = normalized direction (from light) .w = shadowBias
  float4 color;     // .rgb = color, .a = intensity
  LightType type;
  int shadowMapId;      // index to a samplerArray and viewProjections
  float innerConeAngle; // [spot] in radians
  float outerConeAngle; // [spot] in radians
};
// clang-format on

struct LightContribution {
  float3 diffuse;
  float3 specular;
};

// -- FUNCTIONS:

float _getLightRange(const in Light light) { return light.position.w; }
float _getShadowBias(const in Light light) { return light.direction.w; }

float _getLightAttenuation(const in Light light, const in float3 fragToLight) {
  if (light.type == LightType::Directional) return 1.0;

  const float d = length(fragToLight);
  const float lightRange = _getLightRange(light);
  if (d > lightRange) return 0.0;

  // https://github.com/KhronosGroup/glTF/blob/96e14527fb7e2514775b89007b7c1f78b1a67cc1/extensions/2.0/Khronos/KHR_lights_punctual/README.md
  const float rangeAttenuation =
    max(min(1.0 - pow(d / lightRange, 4.0), 1.0), 0.0) / pow(d, 2.0);

  float spotAttenuation = 1.0;
  if (light.type == LightType::Spot) {
    const float innerConeCos = cos(light.innerConeAngle);
    const float outerConeCos = cos(light.outerConeAngle);
    const float actualCos =
      dot(normalize(light.direction.xyz), normalize(-fragToLight));
    if (actualCos > outerConeCos) {
      if (actualCos < innerConeCos)
        spotAttenuation = smoothstep(outerConeCos, innerConeCos, actualCos);
    } else
      spotAttenuation = 0.0;
  }

  return rangeAttenuation * spotAttenuation;
}

float3 _getLightIntensity(const in Light light, const in float3 fragToLight) {
  return light.color.rgb * light.color.a *
         _getLightAttenuation(light, fragToLight);
}

float3 _getFragToLight(const in Light light, const in float3 fragPos) {
  return light.type == LightType::Directional ? -light.direction.xyz
                                              : light.position.xyz - fragPos;
}
