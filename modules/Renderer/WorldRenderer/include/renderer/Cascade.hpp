#pragma once

#include "math/AABB.hpp"
#include "RawCamera.hpp"

namespace gfx {

// Do not use more than 4.
constexpr auto kMaxNumCascades = 4u;

struct Cascade {
  float splitDepth{0.0f};
  AABB aabb;
  RawCamera lightView;
};

} // namespace gfx
