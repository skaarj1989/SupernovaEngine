#pragma once

#include "fg/FrameGraphResource.hpp"

namespace gfx {

struct ReflectiveShadowMapData {
  FrameGraphResource depth;
  FrameGraphResource position;
  FrameGraphResource normal;
  FrameGraphResource flux;
};

} // namespace gfx
