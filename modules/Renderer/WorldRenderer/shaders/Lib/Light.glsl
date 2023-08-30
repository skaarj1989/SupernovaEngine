#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

const uint LightType_Directional = 0;
const uint LightType_Spot = 1;
const uint LightType_Point = 2;

// UploadLights.cpp
struct Light {
  vec4 position;  // [point/spot] .xyz = position, .w = range
  vec4 direction; // [spot/directional] .xyz = normalized direction (from light)
                  // .w = shadowBias
  vec4 color;     // .rgb = color, .a = intensity
  uint type;      // LightType_
  int shadowMapId;      // index to a samplerArray and viewProjections
  float innerConeAngle; // [spot] in radians
  float outerConeAngle; // [spot] in radians
};

#define _DECLARE_LIGHT_BUFFER(S, Index, Name)                                  \
  layout(set = S, binding = Index, std430) buffer readonly _LightBuffer {      \
    uint numLights;                                                            \
    uint _pad[3];                                                              \
    Light data[];                                                              \
  }                                                                            \
  Name

struct LightContribution {
  vec3 diffuse;
  vec3 specular;
};

// -- FUNCTIONS:

float _getLightRange(const in Light light) { return light.position.w; }
float _getShadowBias(const in Light light) { return light.direction.w; }

float _getLightAttenuation(const in Light light, const in vec3 fragToLight) {
  if (light.type == LightType_Directional) return 1.0;

  const float d = length(fragToLight);
  const float lightRange = _getLightRange(light);
  if (d > lightRange) return 0.0;

  // https://github.com/KhronosGroup/glTF/blob/96e14527fb7e2514775b89007b7c1f78b1a67cc1/extensions/2.0/Khronos/KHR_lights_punctual/README.md
  const float rangeAttenuation =
    max(min(1.0 - pow(d / lightRange, 4.0), 1.0), 0.0) / pow(d, 2.0);

  float spotAttenuation = 1.0;
  if (light.type == LightType_Spot) {
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

vec3 _getLightIntensity(const in Light light, const in vec3 fragToLight) {
  return light.color.rgb * light.color.a *
         _getLightAttenuation(light, fragToLight);
}

vec3 _getFragToLight(const in Light light, const in vec3 fragPos) {
  return light.type == LightType_Directional ? -light.direction.xyz
                                             : light.position.xyz - fragPos;
}

#endif
