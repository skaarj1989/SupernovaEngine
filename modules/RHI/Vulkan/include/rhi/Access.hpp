#pragma once

#include "ScopedEnumFlags.hpp"
#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap7.html#synchronization-access-types
enum class Access : VkAccessFlags2 {
  None = VK_ACCESS_2_NONE,

  IndexRead = VK_ACCESS_2_INDEX_READ_BIT,
  VertexAttributeRead = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
  UniformRead = VK_ACCESS_2_UNIFORM_READ_BIT,
  ShaderRead = VK_ACCESS_2_SHADER_READ_BIT,
  ShaderWrite = VK_ACCESS_2_SHADER_WRITE_BIT,
  ColorAttachmentRead = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
  ColorAttachmentWrite = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
  DepthStencilAttachmentRead = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
  DepthStencilAttachmentWrite = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  TransferRead = VK_ACCESS_2_TRANSFER_READ_BIT,
  TransferWrite = VK_ACCESS_2_TRANSFER_WRITE_BIT,
  MemoryRead = VK_ACCESS_2_MEMORY_READ_BIT,
  MemoryWrite = VK_ACCESS_2_MEMORY_WRITE_BIT,
  ShaderStorageRead = VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
  ShaderStorageWrite = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT,
};
template <> struct has_flags<Access> : std::true_type {};

} // namespace rhi
