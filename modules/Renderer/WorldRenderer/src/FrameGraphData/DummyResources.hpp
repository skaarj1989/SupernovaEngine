#pragma once

#include "fg/Fwd.hpp"

namespace gfx {

struct DummyResourcesData {
  FrameGraphResource uniformBuffer;
  FrameGraphResource storageBuffer;

  FrameGraphResource texture2D;
  FrameGraphResource cubeMap;
  FrameGraphResource shadowMaps2DArray;
  FrameGraphResource shadowMapsCubeArray;
};

} // namespace gfx
