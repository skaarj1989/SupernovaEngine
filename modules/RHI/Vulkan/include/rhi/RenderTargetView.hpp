#pragma once

#include "rhi/Texture.hpp"

namespace rhi {

struct RenderTargetView {
  const int32_t frameIndex{0}; // Image in flight index.
  Texture &texture;
};

} // namespace rhi
