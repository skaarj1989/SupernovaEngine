#include "RenderTargetPreview.hpp"
#include "TexturePreview.hpp"

RenderTargetPreview::RenderTargetPreview(rhi::RenderDevice &rd)
    : m_renderDevice{rd} {}

glm::ivec2 RenderTargetPreview::getPosition() const { return m_position; }
rhi::Extent2D RenderTargetPreview::getExtent() const {
  return m_target.getExtent();
}

//
// (private):
//

bool RenderTargetPreview::_isAreaValid(glm::uvec2 area) const {
  const auto maxSize = m_renderDevice.getDeviceLimits().maxImageDimension2D;
  return glm::all(glm::greaterThan(area, glm::uvec2{0})) &&
         glm::all(glm::lessThan(area, glm::uvec2{maxSize}));
}

void RenderTargetPreview::_resize(glm::uvec2 contentSize) {
  m_renderDevice.pushGarbage(m_target);

  using enum rhi::ImageUsage;
  m_target =
    rhi::Texture::Builder{}
      .setExtent({contentSize.x, contentSize.y})
      .setPixelFormat(rhi::PixelFormat::BGRA8_UNorm)
      .setNumMipLevels(1)
      .setNumLayers(std::nullopt)
      .setUsageFlags(TransferDst /* For blit. */ | RenderTarget | Sampled)
      .setupOptimalSampler(true)
      .build(m_renderDevice);
}

void RenderTargetPreview::_present(glm::vec2 size) {
  m_position = glm::vec2{ImGui::GetWindowPos()};
  ::preview(&m_target, size);
}
