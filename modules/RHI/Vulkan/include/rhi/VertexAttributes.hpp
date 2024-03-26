#pragma once

#include "ResourceIndices.hpp"
#include "glad/vulkan.h"
#include <map>

namespace rhi {

struct VertexAttribute {
  enum class Type {
    Float = VK_FORMAT_R32_SFLOAT,
    Float2 = VK_FORMAT_R32G32_SFLOAT,
    Float3 = VK_FORMAT_R32G32B32_SFLOAT,
    Float4 = VK_FORMAT_R32G32B32A32_SFLOAT,

    Int4 = VK_FORMAT_R32G32B32A32_SINT,

    UByte4_Norm = VK_FORMAT_R8G8B8A8_UNORM,
  };
  Type type;
  uint32_t offset{0};
};

// layout(location = index)
using VertexAttributes = std::map<LocationIndex, VertexAttribute>;

[[nodiscard]] uint32_t getSize(const VertexAttribute::Type);

} // namespace rhi
