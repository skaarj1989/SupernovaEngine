#pragma once

#include "rhi/Texture.hpp"
#include "rhi/Rect2D.hpp"
#include "glm/ext/vector_float4.hpp"

namespace rhi {

using ClearValue = std::variant<glm::vec4, float, uint32_t>;

class Texture;

struct AttachmentInfo {
  Texture *target{nullptr};
  std::optional<uint32_t> layer{};
  std::optional<CubeFace> face{};
  std::optional<ClearValue> clearValue{};
};

struct FramebufferInfo {
  Rect2D area;
  uint32_t layers{1};
  std::optional<AttachmentInfo> depthAttachment{std::nullopt};
  bool depthReadOnly{false};
  std::optional<AttachmentInfo> stencilAttachment{std::nullopt};
  bool stencilReadOnly{false};
  std::vector<AttachmentInfo> colorAttachments;
};

[[nodiscard]] inline auto getDepthFormat(const FramebufferInfo &info) {
  return info.depthAttachment ? info.depthAttachment->target->getPixelFormat()
                              : PixelFormat::Undefined;
}
[[nodiscard]] inline auto getColorFormat(const FramebufferInfo &info,
                                         int32_t index) {
  assert(index >= 0 && index < info.colorAttachments.size());
  return info.colorAttachments[index].target->getPixelFormat();
}

[[nodiscard]] std::vector<PixelFormat> getColorFormats(const FramebufferInfo &);

} // namespace rhi
