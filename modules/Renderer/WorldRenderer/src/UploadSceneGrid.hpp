#pragma once

#include "fg/Fwd.hpp"

namespace gfx {

struct Grid;

[[nodiscard]] FrameGraphResource uploadSceneGrid(FrameGraph &, const Grid &);

} // namespace gfx
