#version 460 core

// Use with Mesh.vert

#include "VertexData.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Material.glsl"

layout(push_constant) uniform _PushConstants {
  layout(offset = 16) vec3 u_LightIntensity;
};

#if BLEND_MODE == BLEND_MODE_OPAQUE
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
layout(early_fragment_tests) in;
#endif

layout(location = 0) out vec4 GBuffer0; // .rgb = world position
layout(location = 1) out vec4 GBuffer1; // .rgb = normal
layout(location = 2) out vec4 GBuffer2; // .rgb = flux

void main() {
  Material material = _initMaterial();
  // clang-format off
  _executeUserCode(
    buildFragPosFromWorldSpace(fs_in.fragPosWorldSpace),
#if HAS_PROPERTIES
    u_Properties[fs_in.materialId],
#endif
    material
  );
  // clang-format on

#if BLEND_MODE == BLEND_MODE_MASKED
  if (material.opacity <= 0.0) discard;
#endif

  GBuffer0 = fs_in.fragPosWorldSpace;
  GBuffer1 = vec4(normalize(material.normal), 1.0);
  const vec3 flux = u_LightIntensity * material.baseColor;
  GBuffer2 = vec4(flux, 1.0);
}
