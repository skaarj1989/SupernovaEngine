#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap34.html#VkFormat
enum class PixelFormat {
  Undefined = 0,

  //
  // 8 bits per component:
  //

  // -- Normalized float:

  R8_UNorm = VK_FORMAT_R8_UNORM,
  RG8_UNorm = VK_FORMAT_R8G8_UNORM,
  RGBA8_UNorm = VK_FORMAT_R8G8B8A8_UNORM,
  RGBA8_sRGB = VK_FORMAT_R8G8B8A8_SRGB,

  R8_SNorm = VK_FORMAT_R8_SNORM,
  RG8_SNorm = VK_FORMAT_R8G8_SNORM,
  RGBA8_SNorm = VK_FORMAT_R8G8B8A8_SNORM,

  BGRA8_UNorm = VK_FORMAT_B8G8R8A8_UNORM,
  BGRA8_sRGB = VK_FORMAT_B8G8R8A8_SRGB,

  // -- Integer:

  R8UI = VK_FORMAT_R8_UINT,
  RG8UI = VK_FORMAT_R8G8_UINT,
  RGBA8UI = VK_FORMAT_R8G8B8A8_UINT,

  R8I = VK_FORMAT_R8_SINT,
  RG8I = VK_FORMAT_R8G8_SINT,
  RGBA8I = VK_FORMAT_R8G8B8A8_SINT,

  //
  // 16 bits per component:
  //

  // -- Normalized float:

  R16_UNorm = VK_FORMAT_R16_UNORM,
  RG16_UNorm = VK_FORMAT_R16G16_UNORM,
  RGBA16_UNorm = VK_FORMAT_R16G16B16A16_UNORM,

  R16_SNorm = VK_FORMAT_R16_SNORM,
  RG16_SNorm = VK_FORMAT_R16G16_SNORM,
  RGBA16_SNorm = VK_FORMAT_R16G16B16A16_SNORM,

  // -- Float:

  R16F = VK_FORMAT_R16_SFLOAT,
  RG16F = VK_FORMAT_R16G16_SFLOAT,
  RGBA16F = VK_FORMAT_R16G16B16A16_SFLOAT,

  // -- Integer:

  R16UI = VK_FORMAT_R16_UINT,
  RG16UI = VK_FORMAT_R16G16_UINT,
  RGBA16UI = VK_FORMAT_R16G16B16A16_UINT,

  R16I = VK_FORMAT_R16_SINT,
  RG16I = VK_FORMAT_R16G16_SINT,
  RGBA16I = VK_FORMAT_R16G16B16A16_SINT,

  //
  // 32 bits per component:
  //

  // -- Float:

  R32F = VK_FORMAT_R32_SFLOAT,
  RG32F = VK_FORMAT_R32G32_SFLOAT,
  RGBA32F = VK_FORMAT_R32G32B32A32_SFLOAT,

  // -- Integer:

  R32UI = VK_FORMAT_R32_UINT,
  RG32UI = VK_FORMAT_R32G32_UINT,
  RGBA32UI = VK_FORMAT_R32G32B32A32_UINT,

  R32I = VK_FORMAT_R32_SINT,
  RG32I = VK_FORMAT_R32G32_SINT,
  RGBA32I = VK_FORMAT_R32G32B32A32_SINT,

  //
  // ETC:
  //

  ETC2_RGB8_UNorm = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
  ETC2_RGBA8_UNorm = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
  ETC2_RGB8A1_UNorm = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,

  ETC2_RGB8_sRGB = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
  ETC2_RGBA8_sRGB = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
  ETC2_RGB8A1_sRGB = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,

  BC2_UNorm = VK_FORMAT_BC2_UNORM_BLOCK,
  BC2_sRGB = VK_FORMAT_BC2_SRGB_BLOCK,

  //
  // BC3:
  //

  BC3_UNorm = VK_FORMAT_BC3_UNORM_BLOCK,
  BC3_sRGB = VK_FORMAT_BC3_SRGB_BLOCK,

  //
  // Depth/Stencil:
  //

  // Attachment/Sampled/Blit = ~100%
  // Transfer = ~90%
  Depth16 = VK_FORMAT_D16_UNORM,
  // Sampled/Blit = ~100%
  // Attachment/Transfer/Blit = ~90%
  Depth32F = VK_FORMAT_D32_SFLOAT,

  // Sampled/Blit = ~75%
  // Attachment/Transfer = ~70%
  Stencil8 = VK_FORMAT_S8_UINT,

  // Attachment/Sampled/Transfer = ~24%
  Depth16_Stencil8 = VK_FORMAT_D16_UNORM_S8_UINT,
  // Attachment/Sampled/Blit = ~75%
  // Transfer = ~66%
  Depth24_Stencil8 = VK_FORMAT_D24_UNORM_S8_UINT,
  // Attachment/Sampled/Blit = ~72%
  // Transfer = ~66%
  Depth32F_Stencil8 = VK_FORMAT_D32_SFLOAT_S8_UINT
};

[[nodiscard]] VkImageAspectFlags getAspectMask(const PixelFormat);
[[nodiscard]] const char *toString(const PixelFormat);

} // namespace rhi
