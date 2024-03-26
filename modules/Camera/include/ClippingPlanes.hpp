#pragma once

#include "glm/fwd.hpp"
#include <compare>

namespace gfx {

struct ClippingPlanes {
  float zNear;
  float zFar;

  auto operator<=>(const ClippingPlanes &) const = default;

  template <class Archive> void serialize(Archive &archive) {
    archive(zNear, zFar);
  }
};

[[nodiscard]] ClippingPlanes decomposeProjection(const glm::mat4 &projection);

} // namespace gfx
