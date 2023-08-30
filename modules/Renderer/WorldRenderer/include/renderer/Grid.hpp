#pragma once

#include "math/AABB.hpp"
#include "glm/ext/vector_uint3.hpp"

namespace gfx {

struct Grid {
  explicit Grid(const AABB &);

  [[nodiscard]] bool valid() const;

  AABB aabb;
  glm::uvec3 size;
  float cellSize;
};

} // namespace gfx
