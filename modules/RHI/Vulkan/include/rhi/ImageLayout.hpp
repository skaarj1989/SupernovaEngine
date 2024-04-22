#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap12.html#VkImageLayout
enum class ImageLayout {
  Undefined = VK_IMAGE_LAYOUT_UNDEFINED,

  General = VK_IMAGE_LAYOUT_GENERAL,
  Attachment = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
  ReadOnly = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,

  TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

  Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
};

} // namespace rhi
