#pragma once

#include "glm/ext/matrix_float4x4.hpp"

namespace gfx {

struct RawCamera {
  glm::mat4 view{1.0f};
  glm::mat4 projection{1.0f};
  [[nodiscard]] auto viewProjection() const { return projection * view; }
};

} // namespace gfx
