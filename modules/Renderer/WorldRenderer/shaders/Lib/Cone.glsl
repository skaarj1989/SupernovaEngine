#ifndef _CONE_GLSL_
#define _CONE_GLSL_

struct Cone {
  vec3 T;  // Cone tip
  float h; // Height of the cone
  vec3 d;  // Direction of the cone
  float r; // Bottom radius of the cone
};

#endif
