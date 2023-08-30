#pragma once

#include "renderer/Material.hpp"

namespace gfx {

[[nodiscard]] std::vector<std::byte>
buildPropertyBuffer(const PropertyLayout &, const std::vector<Property> &,
                    VkDeviceSize minOffsetAlignment);

} // namespace gfx
