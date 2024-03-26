#pragma once

#include "ScopedEnumFlags.hpp"
#include <string>

namespace rhi {

enum class ImageUsage {
  TransferSrc = 1 << 0,
  TransferDst = 1 << 1,
  Transfer = TransferSrc | TransferDst,
  Storage = 1 << 2,
  RenderTarget = 1 << 3,
  Sampled = 1 << 4,
};

[[nodiscard]] std::string toString(const ImageUsage);

} // namespace rhi

template <> struct has_flags<rhi::ImageUsage> : std::true_type {};
