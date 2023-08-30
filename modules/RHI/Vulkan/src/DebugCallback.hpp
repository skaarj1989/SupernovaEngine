#pragma once

#include "glad/vulkan.h"

namespace rhi {

[[nodiscard]] VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance);

} // namespace rhi
