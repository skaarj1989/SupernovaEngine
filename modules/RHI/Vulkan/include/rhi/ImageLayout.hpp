#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap12.html#VkImageLayout
enum class ImageLayout {
  Undefined = VK_IMAGE_LAYOUT_UNDEFINED,

  General = VK_IMAGE_LAYOUT_GENERAL,

  ColorAttachment = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  DepthAttachment = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
  DepthStencilAttachment = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  DepthAttachment_StencilReadOnly =
    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
  DepthReadOnly_StencilAttachment =
    VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
  DepthReadOnly = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
  StencilAttachment = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
  StencilReadOnly = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
  DepthStencilReadOnly = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,

  ShaderReadOnly = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,

  TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

  Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
};

} // namespace rhi
