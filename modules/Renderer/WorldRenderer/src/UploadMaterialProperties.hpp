#pragma once

#include "fg/Fwd.hpp"
#include <vector>
#include <optional>

namespace gfx {

void uploadMaterialProperties(FrameGraph &, FrameGraphBlackboard &,
                              std::vector<std::byte> &&);
[[nodiscard]] std::optional<FrameGraphResource>
uploadMaterialProperties(FrameGraph &, std::vector<std::byte> &&);

} // namespace gfx
