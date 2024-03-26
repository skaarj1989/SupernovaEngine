#pragma once

#include "FrameIndex.hpp"

namespace rhi {

class Texture;

struct RenderTargetView {
  const FrameIndex::ValueType frameIndex{0}; // Image in flight index.
  Texture &texture;
};

} // namespace rhi
