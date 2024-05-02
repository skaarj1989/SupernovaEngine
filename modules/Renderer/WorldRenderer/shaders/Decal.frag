#version 460 core

#undef HAS_TANGENTS

#include "VertexData.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Lib/Depth.glsl"

struct BaseAttributes {
  vec3 fragPosWorldSpace;
  vec2 texCoord0;
  vec3 normal;
} g_BaseAttributes;

vec3 _getLocalSpacePos(vec3 fragPosWorldSpace) {
  const vec4 fragPosLocalSpace =
    fs_in.invModelMatrix * vec4(fragPosWorldSpace, 1.0);
  return fragPosLocalSpace.xyz;
}

vec3 getPosition() { return g_BaseAttributes.fragPosWorldSpace; }
vec3 getNormal() { return g_BaseAttributes.normal; }
vec3 getColor() { return vec3(0.0); }
vec2 getTexCoord0() { return g_BaseAttributes.texCoord0; }
vec2 getTexCoord1() { return vec2(0.0); }

layout(set = 1, binding = 5) uniform sampler2D t_SceneDepth;

#include "Material.glsl"

layout(location = 0) out vec4 GBuffer0; // .rgb = Normal, .a = UNUSED
layout(location = 1) out vec4 GBuffer1; // .rgb = Emissive, .a = UNUSED
layout(location = 2) out vec4 GBuffer2; // .rgb = Albedo, .a = SpecularWeight
// .r = Metallic, .g = Roughness, .b = AO, .a = UNUSED
layout(location = 3) out vec4 GBuffer3;
#if WRITE_USERDATA
layout(location = 4) out uint UserData;
#endif

void main() {
  const vec2 ndc = fs_in.fragPosClipSpace.xy / fs_in.fragPosClipSpace.w;
  vec2 depthTexCoord = ndc * 0.5 + 0.5;
  depthTexCoord += getScreenTexelSize() * 0.5;

  const FragmentPosition fragPos =
    buildFragPosFromDepthSampler(t_SceneDepth, depthTexCoord);

  const vec3 fragPosLocalSpace = _getLocalSpacePos(fragPos.worldSpace);

  // Cube half extents = 0.5
  const bvec3 inBox = lessThan(abs(fragPosLocalSpace), vec3(0.5));
  if (!all(inBox)) discard;

  // --

  g_BaseAttributes.fragPosWorldSpace = fragPos.worldSpace;

  vec2 texCoord = fragPosLocalSpace.xy + 0.5;
  texCoord.y = 1.0 - texCoord.y;
  g_BaseAttributes.texCoord0 = texCoord;

  const vec3 dp1 = dFdx(fragPos.worldSpace);
  const vec3 dp2 = dFdy(fragPos.worldSpace);
  g_BaseAttributes.normal = normalize(cross(dp2, dp1));

  Material material = _initMaterial();
  // clang-format off
  _executeUserCode(
    fragPos,
#if HAS_PROPERTIES
    u_Properties[fs_in.materialId],
#endif
    material
  );
  // clang-format off

#if BLEND_MODE == BLEND_MODE_MASKED
  if (material.opacity <= 0.0) discard;
#endif

#if WRITE_NORMAL
  GBuffer0 = vec4(normalize(material.normal), 1.0);
#endif
#if WRITE_EMISSIVE
  GBuffer1 = vec4(material.emissiveColor, 1.0);
#endif
#if WRITE_ALBEDO
  GBuffer2 = vec4(material.baseColor, material.specular);
#endif
#if WRITE_METALLIC_ROUGHNESS_AO
  GBuffer3 = vec4(clamp01(material.metallic), clamp01(material.roughness),
                  clamp01(material.ambientOcclusion), 1.0);
#endif
#if WRITE_USERDATA
  UserData = fs_in.userData;
#endif
}
