#ifndef _MATERIAL_GLSL_
#define _MATERIAL_GLSL_

#ifndef GL_FRAGMENT_SHADER
#  error "FragmentShader only!"
#endif

// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_transmission/README.md#properties
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_volume/README.md#properties

#include "MaterialDefines.glsl"

struct Material {
  vec3 baseColor; // [0..1]
  float opacity;  // MASKED/TRANSPARENT [0..1]

  float ior; // [1..2]

  float transmissionFactor; // TRANSMISSION [0..1]

  // -- VOLUME

  float thickness; // [0..+inf]
  vec3 attenuationColor;
  float attenuationDistance; // [0..+inf]

  // --

  vec3 normal;

  // -- METALLIC-ROUGHNESS

  float metallic;  // [0..1]
  float roughness; // Perceptual [0..1]

  float specular; // [0..1]
  vec3 emissiveColor;
  float ambientOcclusion; // OPAQUE/MASKED [0..1]
};

Material _initMaterial() {
  Material material;
  material.baseColor = getColor();
  material.opacity = 1.0;

  // The default index of refraction of 1.5 yields a dielectric normal
  // incidence reflectance of 0.04.
  material.ior = 1.5;

  material.transmissionFactor = 0.0;

  material.thickness = 0.0;
  material.attenuationColor = vec3(1.0);
  material.attenuationDistance = 0.0;

  material.normal = getNormal();

  material.metallic = 1.0;
  material.roughness = 1.0;

  material.specular = 1.0;
  material.emissiveColor = vec3(0.0);
  material.ambientOcclusion = 1.0;
  return material;
}

#include "UserCodeCommons.glsl"

#include "FragmentPosition.glsl"

#include "Resources/PropertyBuffer.glsl"
_DECLARE_PROPERTIES(0, 1);

#region USER_SAMPLERS

#region USER_MODULES

// clang-format off
void _executeUserCode(
  FragmentPosition fragPos,
#if HAS_PROPERTIES
  in Properties properties,
#endif
  inout Material material) {
#region USER_CODE
}
// clang-format on

#endif
