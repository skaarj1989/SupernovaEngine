#ifndef _HISTOGRAM_BUFFER_GLSL_
#define _HISTOGRAM_BUFFER_GLSL_

layout(set = 0, binding = 1, std430) buffer _Histogram {
  uint g_Histogram[]; // ArrayStride = 4
};

#endif
