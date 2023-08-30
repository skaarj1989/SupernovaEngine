#pragma once

#include "fg/Fwd.hpp"
#include "renderer/Grid.hpp"

namespace gfx {

[[nodiscard]] FrameGraphResource uploadSceneGrid(FrameGraph &, const Grid &);

} // namespace gfx
