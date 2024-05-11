#pragma once

#include "DataType.hpp"
#include "rhi/Texture.hpp"
#include <memory>

struct TextureParam {
  std::shared_ptr<rhi::Texture> texture;
};
inline bool operator==(const TextureParam &lhs, const TextureParam &rhs) {
  return lhs.texture == rhs.texture;
}

[[nodiscard]] DataType getDataType(const TextureParam &);
