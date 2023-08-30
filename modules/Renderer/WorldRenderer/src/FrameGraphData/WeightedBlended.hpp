#pragma once

#include "fg/FrameGraphResource.hpp"

namespace gfx {

struct WeightedBlendedData {
  FrameGraphResource accum;
  FrameGraphResource reveal;
};

} // namespace gfx
