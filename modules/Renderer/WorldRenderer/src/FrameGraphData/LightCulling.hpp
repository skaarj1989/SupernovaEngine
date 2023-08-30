#pragma once

#include "fg/FrameGraphResource.hpp"
#include <optional>

namespace gfx {

using TileSize = uint32_t;

struct LightCullingData {
  TileSize tileSize;
  FrameGraphResource lightIndices;
  FrameGraphResource lightGrid;
  std::optional<FrameGraphResource> debugMap;
};

} // namespace gfx
