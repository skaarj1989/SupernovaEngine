#pragma once

#include "fg/Fwd.hpp"
#include "GPUInstance.hpp"
#include <vector>
#include <optional>

namespace gfx {

[[nodiscard]] std::optional<FrameGraphResource>
uploadInstances(FrameGraph &, std::vector<GPUInstance> &&);

} // namespace gfx
