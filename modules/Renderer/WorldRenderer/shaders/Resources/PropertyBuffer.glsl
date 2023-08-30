#ifndef _PROPERTY_BUFFER_GLSL_
#define _PROPERTY_BUFFER_GLSL_

struct Properties {
#region USER_PROPERTIES
#if !HAS_PROPERTIES
  uint _dummy;
#endif
};

#define _DECLARE_PROPERTIES(S, Index)                                          \
  layout(set = S, binding = Index, std430) buffer readonly _PropertyBuffer {   \
    Properties u_Properties[];                                                 \
  }

#endif
