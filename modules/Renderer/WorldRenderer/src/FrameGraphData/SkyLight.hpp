#pragma once

#include "fg/FrameGraphResource.hpp"

namespace gfx {

struct SkyLightData {
  FrameGraphResource environment; // For skybox
  FrameGraphResource diffuse;     // Irradiance
  FrameGraphResource specular;    // Prefiltered environment map
};

} // namespace gfx
