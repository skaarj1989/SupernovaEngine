#ifndef _GBUFFER_HELPER_GLSL_
#define _GBUFFER_HELPER_GLSL_

/*
 | 2 bits, 0.. 1 | 6 bits, 2.. 7 |
 |  shadingModel | materialFlags |
*/

float encodeMisc(uint shadingModel, uint flags) {
  uint encoded = bitfieldInsert(0, shadingModel, 0, 2);
  encoded = bitfieldInsert(encoded, flags, 2, 6);
  const float kNorm = 1.0 / 255;
  return float(encoded) * kNorm;
}

uint sampleEncodedMiscData(texture2D t, sampler s, vec2 texCoord) {
  const float misc = texture(sampler2D(t, s), texCoord).r;
  return uint(misc * 255.0);
}

uint decodeShadingModel(uint encoded) { return bitfieldExtract(encoded, 0, 2); }
uint decodeMaterialFlags(uint encoded) {
  return bitfieldExtract(encoded, 2, 6);
}

#endif
