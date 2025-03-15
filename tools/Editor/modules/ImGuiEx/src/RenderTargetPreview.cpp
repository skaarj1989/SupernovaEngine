#include "RenderTargetPreview.hpp"
#include "rhi/Extent2D.hpp"
#include "rhi/RenderDevice.hpp"
#include "TexturePreview.hpp"

RenderTargetPreview::RenderTargetPreview(rhi::RenderDevice &rd)
    : m_renderDevice{rd} {}

glm::ivec2 RenderTargetPreview::getPosition() const { return m_position; }
rhi::Extent2D RenderTargetPreview::getExtent() const {
  return m_target ? m_target->getExtent() : rhi::Extent2D{0, 0};
}

//
// (private):
//

bool RenderTargetPreview::_isAreaValid(const glm::uvec2 area) const {
  const auto maxSize = m_renderDevice.getDeviceLimits().maxImageDimension2D;
  return glm::all(glm::greaterThan(area, glm::uvec2{0})) &&
         glm::all(glm::lessThan(area, glm::uvec2{maxSize}));
}

void RenderTargetPreview::_resize(const glm::uvec2 contentSize) {
  using enum rhi::ImageUsage;

  rhi::Texture::Builder builder;
  builder.setExtent({contentSize.x, contentSize.y})
      .setPixelFormat(rhi::PixelFormat::BGRA8_UNorm)
      .setNumMipLevels(1)
      .setNumLayers(std::nullopt)
      .setUsageFlags(TransferDst /* For blit. */ | RenderTarget | Sampled)
    .setupOptimalSampler(true);

  m_target = rhi::buildShared(m_renderDevice, builder);
}

void RenderTargetPreview::_present(const glm::vec2 size) {
  m_position = glm::vec2{ImGui::GetWindowPos()};
  ::preview(m_target.get(), size);
}
