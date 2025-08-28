#pragma once

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

uint sampleEncodedMiscData(Texture2D t, SamplerState s, float2 texCoord) {
  const float misc = t.Sample(s, texCoord).r;
  return uint(misc * 255.0);
}

uint decodeShadingModel(uint encoded) { return bitfieldExtract(encoded, 0, 2); }
uint decodeMaterialFlags(uint encoded) {
  return bitfieldExtract(encoded, 2, 6);
}
