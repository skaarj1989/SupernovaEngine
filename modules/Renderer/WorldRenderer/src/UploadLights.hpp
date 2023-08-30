#pragma once

#include "fg/Fwd.hpp"
#include "renderer/Light.hpp"
#include "robin_hood.h"

namespace gfx {

using LightShadowPair = robin_hood::pair<const Light *, int32_t>;
using ShadowMapIndices =
  robin_hood::unordered_map<LightType, std::vector<LightShadowPair>>;

void uploadLights(FrameGraph &, FrameGraphBlackboard &,
                  std::vector<const Light *> &&, ShadowMapIndices &&);

} // namespace gfx
