#ifndef _VERTEX_DATA_GLSL_
#define _VERTEX_DATA_GLSL_

#ifdef GL_VERTEX_SHADER
out gl_PerVertex { vec4 gl_Position; };

#  define INTERFACE_BLOCK(x) out x
#  define VERTEX_DATA_BLOCK vs_out
#elif GL_GEOMETRY_SHADER
#  define INTERFACE_BLOCK(x) in x
#  define VERTEX_DATA_BLOCK gs_in[]
#elif GL_FRAGMENT_SHADER
#  define INTERFACE_BLOCK(x) in x
#  define VERTEX_DATA_BLOCK fs_in
#endif

layout(location = 0) INTERFACE_BLOCK(VertexData) {
#ifdef IS_DECAL
  vec4 fragPosClipSpace;
  flat mat4 invModelMatrix; // world -> local
#else
  vec4 fragPosWorldSpace;
#  ifdef HAS_NORMAL
#    ifdef HAS_TANGENTS
  mat3 TBN; // tangent -> world
#    else
  vec3 normal; // in world-space
#    endif
#  endif
#  ifdef HAS_TEXCOORD0
  vec2 texCoord0;
#  endif
#  ifdef HAS_TEXCOORD1
  vec2 texCoord1;
#  endif
#  ifdef HAS_COLOR
  vec3 color;
#  endif

  flat mat4 modelMatrix; // model -> world
#endif

  flat uint materialId;
  flat uint flags;
  flat uint userData;
}
VERTEX_DATA_BLOCK;

#if defined(GL_FRAGMENT_SHADER) && !defined(IS_DECAL)
vec4 getPosition() { return VERTEX_DATA_BLOCK.fragPosWorldSpace; }

vec3 getNormal() {
#  ifdef HAS_NORMAL
  return normalize(
#    ifdef HAS_TANGENTS
    VERTEX_DATA_BLOCK.TBN[2]
#    else
    VERTEX_DATA_BLOCK.normal
#    endif
    * (gl_FrontFacing ? 1.0 : -1.0));
#  else

  return vec3(0.0);
#  endif
}

vec3 getColor() {
#  ifdef HAS_COLOR
  return VERTEX_DATA_BLOCK.color;
#  else
  return vec3(0.0);
#  endif
}

vec2 getTexCoord0() {
#  ifdef HAS_TEXCOORD0
  return VERTEX_DATA_BLOCK.texCoord0;
#  else
  return vec2(0.0);
#  endif
}

vec2 getTexCoord1() {
#  ifdef HAS_TEXCOORD1
  return VERTEX_DATA_BLOCK.texCoord1;
#  else
  return vec2(0.0);
#  endif
}

mat4 getModelMatrix() { return VERTEX_DATA_BLOCK.modelMatrix; }
#endif

#undef INTERFACE_BLOCK
#undef VERTEX_DATA_BLOCK

#endif
