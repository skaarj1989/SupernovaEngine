#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap25.html#VkCullModeFlagBits
enum class CullMode : VkCullModeFlags {
  // Specifies that no triangles are discarded
  None = VK_CULL_MODE_NONE,
  // Specifies that front-facing triangles are discarded
  Front = VK_CULL_MODE_FRONT_BIT,
  // Specifies that back-facing triangles are discarded
  Back = VK_CULL_MODE_BACK_BIT,
};

[[nodiscard]] const char *toString(const CullMode);

} // namespace rhi
