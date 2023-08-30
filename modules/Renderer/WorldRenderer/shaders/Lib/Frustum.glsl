#ifndef _FRUSTUM_GLSL_
#define _FRUSTUM_GLSL_

#include "Plane.glsl"

struct Frustum {
  Plane planes[4];
};

#define _DECLARE_FRUSTUMS(S, Index, Access, Name)                              \
  layout(set = S, binding = Index, std430) Access buffer _GridFrustums {       \
    Frustum Name[];                                                            \
  }

#endif
