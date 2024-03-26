#include "DebugCallback.hpp"
#include "VkCheck.hpp"
#include "spdlog/spdlog.h"
#include <array>
#include <algorithm>

namespace rhi {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *) {
#if 1
  constexpr auto kIgnoredIds = std::array{
    // UNASSIGNED-BestPractices-

    -2027362524, // vkCreateCommandPool-command-buffer-reset
    1413273847,  // BindMemory-NoPriority
    2076578180,  // SpirvDeprecated_WorkgroupSize
    887288624,   // ClearAttachment-ClearImage
    1835555994,  // Pipeline-SortAndBind
    411464045,   // BindPipeline-SwitchTessGeometryMesh
  };
  constexpr auto equals = [](const auto v) {
    return [v](const auto e) { return e == v; };
  };
  if (std::ranges::any_of(kIgnoredIds,
                          equals(pCallbackData->messageIdNumber))) {
    return VK_FALSE;
  }
#endif

  static constexpr auto kMessageFormat = "{}";
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    SPDLOG_ERROR(kMessageFormat, pCallbackData->pMessage);
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    SPDLOG_WARN(kMessageFormat, pCallbackData->pMessage);
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    SPDLOG_INFO(kMessageFormat, pCallbackData->pMessage);
  }
  // VK_TRUE value is reserved for use in layer development.
  return VK_FALSE;
}

} // namespace

VkDebugUtilsMessengerEXT createDebugMessenger(const VkInstance instance) {
  assert(instance != VK_NULL_HANDLE);

  const VkDebugUtilsMessengerCreateInfoEXT createInfo{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debugCallback,
  };
  VkDebugUtilsMessengerEXT debugMessenger;
  VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                          &debugMessenger));
  return debugMessenger;
}

} // namespace rhi
