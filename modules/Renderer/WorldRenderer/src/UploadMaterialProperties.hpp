#pragma once

#include "fg/Fwd.hpp"
#include <vector>

namespace gfx {

[[nodiscard]] FrameGraphResource
uploadMaterialProperties(FrameGraph &, std::vector<std::byte> &&);

} // namespace gfx
