#pragma once

#include "fg/Fwd.hpp"
#include "GPUInstance.hpp"
#include <vector>

namespace gfx {

[[nodiscard]] FrameGraphResource uploadInstances(FrameGraph &,
                                                 std::vector<GPUInstance> &&);

} // namespace gfx
