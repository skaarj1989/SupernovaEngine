#ifndef _HAMMERSLEY2D_GLSL_
#define _HAMMERSLEY2D_GLSL_

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float _radicalInverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 hammersley2D(uint i, uint N) {
  return vec2(float(i) / float(N), _radicalInverse_VdC(i));
}

#endif
