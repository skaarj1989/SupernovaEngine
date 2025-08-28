#pragma once

int2 textureSize(Texture2D texture, uint level = 0) {
  int2 dimensions;
  int _0;
  texture.GetDimensions(level, dimensions.x, dimensions.y, _0);
  return dimensions;
}

int2 textureSize(Sampler2D texture, uint level = 0) {
  int2 dimensions;
  int _0;
  texture.GetDimensions(level, dimensions.x, dimensions.y, _0);
  return dimensions;
}

float2 getTexelSize(Texture2D texture) {
  return 1.0 / float2(textureSize(texture));
}

float2 getTexelSize(Sampler2D texture) {
  return 1.0 / float2(textureSize(texture));
}

// Returns the number of mip levels for a given texture
float calculateMipLevels(Texture2D texture) {
  uint width = textureSize(texture).x;
  return floor(log2(float(width))) + 1.0;
}
