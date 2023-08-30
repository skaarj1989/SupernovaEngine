#pragma once

#include "glad/vulkan.h"

namespace gfx {

struct CommonSamplers {
  VkSampler point{VK_NULL_HANDLE};
  VkSampler bilinear{VK_NULL_HANDLE};
  VkSampler shadow{VK_NULL_HANDLE};
  VkSampler omniShadow{VK_NULL_HANDLE};
};

} // namespace gfx
