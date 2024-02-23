#pragma once

#include "rhi/Texture.hpp"
#include "rhi/FrameIndex.hpp"

namespace rhi {

struct RenderTargetView {
  const FrameIndex::ValueType frameIndex{0}; // Image in flight index.
  Texture &texture;
};

} // namespace rhi
