#include "rhi/FramebufferInfo.hpp"
#include <algorithm> // transform

namespace rhi {

namespace {

[[nodiscard]] auto getColorFormats(std::span<const AttachmentInfo> v) {
  std::vector<rhi::PixelFormat> out(v.size());
  std::ranges::transform(v, out.begin(), [](const auto &attachment) {
    return attachment.target->getPixelFormat();
  });
  return out;
}

} // namespace

std::vector<PixelFormat> getColorFormats(const FramebufferInfo &info) {
  return getColorFormats(info.colorAttachments);
}

} // namespace rhi
