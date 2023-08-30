#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap13.html#VkFilter
enum class TexelFilter {
  Nearest = VK_FILTER_NEAREST,
  Linear = VK_FILTER_LINEAR
};

} // namespace rhi
