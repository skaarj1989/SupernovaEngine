#version 460 core

// Use with Mesh.vert

#include "VertexData.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Material.glsl"

#if BLEND_MODE == BLEND_MODE_OPAQUE
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
layout(early_fragment_tests) in;
#endif

void main() {
  Material material = _initMaterial();
#if BLEND_MODE == BLEND_MODE_MASKED
  // clang-format off
  _executeUserCode(
    buildFragPosFromWorldSpace(fs_in.fragPosWorldSpace),
#  if HAS_PROPERTIES
    u_Properties[fs_in.materialId],
#  endif
    material
  );
  // clang-format on
#endif

#if BLEND_MODE == BLEND_MODE_MASKED
  if (material.opacity <= 0.0) discard;
#endif
}
