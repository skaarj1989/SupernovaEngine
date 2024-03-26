#pragma once

#include "glad/vulkan.h"

namespace rhi {

[[nodiscard]] VkDebugUtilsMessengerEXT createDebugMessenger(const VkInstance);

} // namespace rhi
