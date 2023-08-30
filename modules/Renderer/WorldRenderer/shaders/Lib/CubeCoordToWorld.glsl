#ifndef _CUBE_COORD_TO_WORLD_HELPER_GLSL_
#define _CUBE_COORD_TO_WORLD_HELPER_GLSL_

#define POSITIVE_X 0
#define NEGATIVE_X 1
#define POSITIVE_Y 2
#define NEGATIVE_Y 3
#define POSITIVE_Z 4
#define NEGATIVE_Z 5

vec3 getCubeFace(int face, vec2 uv) {
  // clang-format off
  switch (face) {
    case POSITIVE_X: return vec3(1.0, -uv.yx);       
    case NEGATIVE_X: return vec3(-1.0, -uv.y, uv.x); 
    case POSITIVE_Y: return vec3(uv.x, 1.0, uv.y);  
    case NEGATIVE_Y: return vec3(uv.x, -1.0, -uv.y); 
    case POSITIVE_Z: return vec3(uv.x, -uv.y, 1.0);  
    case NEGATIVE_Z: return vec3(-uv.xy, -1.0);      
  }
  // clang-format on
  return vec3(0.0);
}

vec3 cubeCoordToWorld(ivec3 cubeCoord, float cubemapSize) {
  const vec2 uv = vec2(cubeCoord.xy) / cubemapSize;
  return getCubeFace(cubeCoord.z, uv * 2.0 - 1.0);
}

#endif
