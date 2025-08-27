#pragma once

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float _radicalInverse_VdC(uint bits) {
  bits = (bits << 16) | (bits >> 16);
  bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
  bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
  bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
  bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
  return float(bits) * 2.3283064365386963e-10; // 0x100000000
}

float2 hammersley2D(uint i, uint N) {
  return float2(float(i) / float(N), _radicalInverse_VdC(i));
}
