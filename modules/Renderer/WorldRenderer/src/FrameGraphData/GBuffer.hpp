#pragma once

#include "fg/FrameGraphResource.hpp"

namespace gfx {

struct GBufferData {
  FrameGraphResource depth;
  FrameGraphResource normal;
  FrameGraphResource emissive;
  FrameGraphResource albedo;
  FrameGraphResource metallicRoughnessAO;
  FrameGraphResource misc; // .r = ShadingModel/MaterialFlags
};

} // namespace gfx
