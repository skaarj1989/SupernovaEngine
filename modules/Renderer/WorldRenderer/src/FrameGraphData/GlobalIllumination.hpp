#pragma once

#include "renderer/FrameGraphData/LightPropagationVolumes.hpp"

namespace gfx {

struct GlobalIlluminationData {
  FrameGraphResource sceneGridBlock;
  LightPropagationVolumesData LPV;
};

} // namespace gfx
