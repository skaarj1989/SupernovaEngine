#pragma once

#include "fg/Fwd.hpp"
#include "glm/fwd.hpp"
#include <vector>

namespace gfx {

void uploadSkins(FrameGraph &, FrameGraphBlackboard &,
                 std::vector<glm::mat4> &&skins);

} // namespace gfx
