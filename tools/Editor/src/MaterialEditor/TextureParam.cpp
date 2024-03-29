#include "MaterialEditor/TextureParam.hpp"

DataType getDataType(const TextureParam &param) {
  using enum DataType;

  if (param.texture && *param.texture) {
    switch (param.texture->getType()) {
      using enum rhi::TextureType;

    case Texture2D:
      return Sampler2D;
    case TextureCube:
      return SamplerCube;

    default:
      break;
    }
  }
  return Undefined;
}
