#pragma once

#include "ScopedEnumFlags.hpp"
#include <string>

namespace gfx {

enum class DebugFlags : int32_t {
  None = 0,

  WorldBounds = 1 << 0,
  InfiniteGrid = 1 << 1,

  Wireframe = 1 << 2,
  VertexNormal = 1 << 3,

  CascadeSplits = 1 << 4,
  LightHeatmap = 1 << 5,

  VPL = 1 << 6,
  IrradianceOnly = 1 << 7,
};

[[nodiscard]] std::string toString(const DebugFlags);

} // namespace gfx

template <> struct has_flags<gfx::DebugFlags> : std::true_type {};
