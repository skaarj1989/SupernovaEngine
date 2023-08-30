#ifndef _VERTEX_ATTRIBUTES_GLSL_
#define _VERTEX_ATTRIBUTES_GLSL_

#ifndef GL_VERTEX_SHADER
#  error "VertexShader only!"
#endif

// WorldRenderer/VertexFormat.hpp

layout(location = 0) in vec3 a_Position;
#ifdef HAS_COLOR
layout(location = 1) in vec3 a_Color0;
#endif
#ifdef HAS_NORMAL
layout(location = 2) in vec3 a_Normal;
#endif
#ifdef HAS_TEXCOORD0
layout(location = 3) in vec2 a_TexCoord0;
#  ifdef HAS_TANGENTS
layout(location = 5) in vec3 a_Tangent;
layout(location = 6) in vec3 a_Bitangent;
#  endif
#endif
#ifdef HAS_TEXCOORD1
layout(location = 4) in vec2 a_TexCoord1;
#endif
#ifdef IS_SKINNED
layout(location = 7) in ivec4 a_Joints;
layout(location = 8) in vec4 a_Weights;
#endif

//
// Getters:
//

vec4 getPosition() { return vec4(a_Position, 1.0); }

vec3 getColor() {
#ifdef HAS_COLOR
  return a_Color0;
#else
  return vec3(0.0);
#endif
}

vec3 getNormal() {
#ifdef HAS_NORMAL
  return a_Normal;
#else
  return vec3(0.0);
#endif
}

vec2 getTexCoord0() {
#ifdef HAS_TEXCOORD0
  return a_TexCoord0;
#else
  return vec2(0.0);
#endif
}
vec2 getTexCoord1() {
#ifdef HAS_TEXCOORD1
  return a_TexCoord1;
#else
  return vec2(0.0);
#endif
}

#endif
