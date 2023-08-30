#pragma once

#include "fg/Fwd.hpp"
#include "glm/fwd.hpp"
#include <vector>

namespace gfx {

void uploadTransforms(FrameGraph &, FrameGraphBlackboard &,
                      std::vector<glm::mat4> &&transforms);

} // namespace gfx
