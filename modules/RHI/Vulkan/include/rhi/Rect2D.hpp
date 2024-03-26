#pragma once

#include "Offset2D.hpp"
#include "Extent2D.hpp"
#include "glm/ext/vector_int4.hpp"

namespace rhi {

struct Rect2D {
  Offset2D offset;
  Extent2D extent;

  [[nodiscard]] explicit operator VkRect2D() const {
    return {offset.x, offset.y, extent.width, extent.height};
  }
  [[nodiscard]] explicit operator glm::ivec4() const {
    return {offset.x, offset.y, extent.width, extent.height};
  }

  auto operator<=>(const Rect2D &) const = default;

  template <class Archive> void serialize(Archive &archive) {
    archive(offset, extent);
  }
};

} // namespace rhi
