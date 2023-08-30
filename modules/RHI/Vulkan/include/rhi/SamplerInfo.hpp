#pragma once

#include "TexelFilter.hpp"
#include "CompareOp.hpp"
#include <optional>

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap13.html#VkSamplerMipmapMode
enum class MipmapMode {
  Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
  Linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
};

// https://registry.khronos.org/vulkan/specs/1.3/html/chap13.html#VkSamplerAddressMode
enum class SamplerAddressMode {
  Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
  ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
  ClampToBorder = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
  MirrorClampToEdge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
};

// https://registry.khronos.org/vulkan/specs/1.3/html/chap13.html#VkBorderColor
enum class BorderColor {
  TransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
  OpaqueBlack = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
  OpaqueWhite = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
};

struct SamplerInfo {
  TexelFilter magFilter{TexelFilter::Linear};
  TexelFilter minFilter{TexelFilter::Nearest};
  MipmapMode mipmapMode{MipmapMode::Linear};

  SamplerAddressMode addressModeS{SamplerAddressMode::Repeat};
  SamplerAddressMode addressModeT{SamplerAddressMode::Repeat};
  SamplerAddressMode addressModeR{SamplerAddressMode::Repeat};

  // The anisotropy value clamp used by the sampler.
  std::optional<float> maxAnisotropy{std::nullopt};
  std::optional<CompareOp> compareOp{std::nullopt};

  // https://registry.khronos.org/vulkan/specs/1.3/html/chap16.html#textures-level-of-detail-operation

  float minLod{0.0f};
  float maxLod{1.0f};

  BorderColor borderColor{BorderColor::OpaqueBlack};
};

} // namespace rhi
