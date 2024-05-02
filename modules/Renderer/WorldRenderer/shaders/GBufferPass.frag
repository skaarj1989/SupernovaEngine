#version 460 core

// Use with Mesh.vert

#include "VertexData.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Material.glsl"

#include "Lib/GBufferHelper.glsl"

#if BLEND_MODE == BLEND_MODE_OPAQUE
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
layout(early_fragment_tests) in;
#endif

layout(location = 0) out vec4 GBuffer0; // .rgb = Normal, .a = UNUSED
layout(location = 1) out vec4 GBuffer1; // .rgb = Emissive, .a = UNUSED
layout(location = 2) out vec4 GBuffer2; // .rgb = Albedo, .a = SpecularWeight
// .r = Metallic, .g = Roughness, .b = AO, .a = UNUSED
layout(location = 3) out vec4 GBuffer3;
layout(location = 4) out float GBuffer4; // .r = ShadingModel/MaterialFlags
#if WRITE_USERDATA
layout(location = 5) out uint UserData;
#endif

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

  GBuffer0 = vec4(normalize(material.normal), 1.0);
  GBuffer1 = vec4(material.emissiveColor, 1.0);
  GBuffer2 = vec4(material.baseColor, material.specular);
  GBuffer3 = vec4(clamp01(material.metallic), clamp01(material.roughness),
                  clamp01(material.ambientOcclusion), 1.0);
  GBuffer4 = encodeMisc(SHADING_MODEL, fs_in.flags);
#if WRITE_USERDATA
  UserData = fs_in.userData;
#endif
}
