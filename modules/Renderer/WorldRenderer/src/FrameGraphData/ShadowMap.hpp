#pragma once

#include "fg/FrameGraphResource.hpp"
#include <optional>

namespace gfx {

struct ShadowMapData {
  std::optional<FrameGraphResource> shadowBlock;
  std::optional<FrameGraphResource> cascadedShadowMaps;
  std::optional<FrameGraphResource> spotLightShadowMaps;
  std::optional<FrameGraphResource> omniShadowMaps;
};

} // namespace gfx
