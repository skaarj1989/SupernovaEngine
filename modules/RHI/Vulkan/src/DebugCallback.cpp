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
  constexpr auto kIgnoredIds = std::array{
    0,
    // BestPractices-
    141128897,   // vkCreateCommandPool-command-buffer-reset
    2011355468,  // CreatePipelineLayout-SeparateSampler
    -2036050732, // DrawState-VtxIndexOutOfBounds
    -567953719,  // BindMemory-NoPriority
    597400213,   // CreateImage-Depth32Format
    -492529551,  // CreatePipelinesLayout-KeepLayoutSmall
    1829508205,  // Pipeline-SortAndBind

    227182348,   // SpirvDeprecated_WorkgroupSize
    1035616080,  // ClearAttachment-ClearImage
    -1442629603, // Submission-ReduceNumberOfSubmissions
  };
  constexpr auto equals = [](const auto in) {
    return [in](const auto ignored) { return in == ignored; };
  };
  if (std::ranges::any_of(kIgnoredIds,
                          equals(pCallbackData->messageIdNumber))) {
    return VK_FALSE;
  }

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
