#ifndef _TONEMAP_UNCHARTED_GLSL_
#define _TONEMAP_UNCHARTED_GLSL_

vec3 partialTonemapUncharted2(vec3 x) {
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
vec3 tonemapUncharted2(vec3 v) {
  const float exposureBias = 2.0;
  vec3 curr = partialTonemapUncharted2(v * exposureBias);

  vec3 W = vec3(11.2);
  const vec3 whiteScale = vec3(1.0) / partialTonemapUncharted2(W);
  return curr * whiteScale;
}

#endif
