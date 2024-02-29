#version 460 core

#include "VertexAttributes.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Lib/DefaultInstance.glsl"
#include "Resources/InstanceBuffer.glsl"
#include "Resources/TransformBuffer.glsl"
#include "Resources/SkinBuffer.glsl"

#include "Resources/PropertyBuffer.glsl"
_DECLARE_PROPERTIES(0, 1);

#region USER_SAMPLERS

#include "Lib/Math.glsl"
#include "Lib/Billboard.glsl"

#include "Lib/Noise.glsl"

#define getModelMatrix() modelMatrix

#region USER_MODULES

// clang-format off
void _executeUserCode(
#if HAS_PROPERTIES
  in Properties properties,
#endif
  in mat4 modelMatrix,
  inout vec4 localPos) {
#region USER_CODE
}
// clang-format on

#include "VertexData.glsl"

#include "TransformMacros.glsl"

#if OUT_VERTEX_VIEW_SPACE
#  define VERTEX_TARGET_SPACE LOCAL_TO_VIEW_SPACE
#elif OUT_VERTEX_CLIP_SPACE
#  define VERTEX_TARGET_SPACE LOCAL_TO_CLIP_SPACE
#else
#  error "Vertex position: undefined target space"
#endif

#ifdef HAS_NORMAL
#  if OUT_NORMAL_WORLD_SPACE
#    define NORMAL_TARGET_SPACE LOCAL_TO_WORLD_SPACE
#  elif OUT_NORMAL_VIEW_SPACE
#    define NORMAL_TARGET_SPACE LOCAL_TO_VIEW_SPACE
#  elif OUT_NORMAL_CLIP_SPACE
#    define NORMAL_TARGET_SPACE LOCAL_TO_CLIP_SPACE
#  else
#    error "Vertex normal: undefined target space"
#  endif
#endif

void main() {
  const Instance instance = GET_INSTANCE();
  vs_out.materialId = instance.materialId;
  vs_out.flags = instance.flags;

  const mat4 modelMatrix = g_Transforms[instance.transformId];
  vec4 localPos = vec4(a_Position, 1.0);

#if !NO_MATERIAL
  // clang-format off
  _executeUserCode(
#if HAS_PROPERTIES
    u_Properties[instance.materialId],
#endif
    modelMatrix, localPos
  );
  // clang-format on
#endif

#ifdef IS_SKINNED
  const mat4 skinMatrix = instance.skinOffset == ~0
                            ? mat4(1.0)
                            : _buildSkinMatrix(instance.skinOffset);
  localPos = skinMatrix * localPos;
#endif

  vs_out.fragPosWorldSpace = modelMatrix * localPos;
  vs_out.modelMatrix = modelMatrix;

#ifdef HAS_NORMAL
#  ifdef IS_SKINNED
  const mat3 normalMatrix =
    _buildNormalMatrix(NORMAL_TARGET_SPACE * skinMatrix);
#  else
  const mat3 normalMatrix = _buildNormalMatrix(NORMAL_TARGET_SPACE);
#  endif
  const vec3 N = normalize(normalMatrix * a_Normal);
#  ifdef HAS_TANGENTS
  vec3 T = normalize(normalMatrix * a_Tangent);
  T = normalize(T - dot(T, N) * N);
  vec3 B = normalize(normalMatrix * a_Bitangent);
  vs_out.TBN = mat3(T, B, N);
#  else
  vs_out.normal = N;
#  endif
#endif

#ifdef HAS_TEXCOORD0
  vs_out.texCoord0 = a_TexCoord0;
#endif
#ifdef HAS_TEXCOORD1
  vs_out.texCoord1 = a_TexCoord1;
#endif

#ifdef HAS_COLOR
  vs_out.color = a_Color0;
#endif

  gl_Position = VERTEX_TARGET_SPACE * localPos;
}
