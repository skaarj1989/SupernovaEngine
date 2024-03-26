#pragma once

#include "glad/vulkan.h"
#include "renderer/MaterialProperty.hpp"
#include <vector>

namespace gfx {

struct PropertyLayout;

[[nodiscard]] std::vector<std::byte>
buildPropertyBuffer(const PropertyLayout &, const std::vector<Property> &,
                    const VkDeviceSize minOffsetAlignment);

} // namespace gfx
