#ifndef _HEATMAP_GLSL_
#define _HEATMAP_GLSL_

// https://stackoverflow.com/a/20792531/4265215

vec3 calculateHeat(float minimum, float maximum, float value) {
  const float kMaxValue = 255.0;
  const float kInvMaxValue = 1.0 / kMaxValue;

  const float ratio = 2.0 * (value - minimum) / (maximum - minimum);
  const float b = max(0.0, kMaxValue * (1.0 - ratio)); // cold
  const float r = max(0.0, kMaxValue * (ratio - 1.0)); // hot
  const float g = kMaxValue - b - r;                   // warm
  return vec3(r, g, b) * kInvMaxValue;
}

#endif
