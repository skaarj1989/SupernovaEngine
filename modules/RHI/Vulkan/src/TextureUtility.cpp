#include "rhi/TextureUtility.hpp"
#include "rhi/RenderDevice.hpp"

namespace rhi {

namespace {

// @return The first mip level of the first layer.
[[nodiscard]] auto getDefaultRegion(const Texture &texture) {
  const auto extent = texture.getExtent();
  return VkBufferImageCopy{
    .imageSubresource =
      {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .layerCount = 1,
      },
    .imageExtent =
      {
        .width = extent.width,
        .height = extent.height,
        .depth = 1,
      },
  };
}

} // namespace

void upload(RenderDevice &rd, const Buffer &srcStagingBuffer,
            std::span<const VkBufferImageCopy> copyRegions, Texture &texture,
            const bool generateMipmaps) {
  rd.execute([&](CommandBuffer &cb) {
    cb.copyBuffer(srcStagingBuffer, texture,
                  copyRegions.empty() ? std::array{getDefaultRegion(texture)}
                                      : copyRegions);
    if (generateMipmaps) cb.generateMipmaps(texture);

    cb.getBarrierBuilder().imageBarrier(
      {
        .image = texture,
        .newLayout = ImageLayout::ReadOnly,
        .subresourceRange =
          {
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
          },
      },
      {
        .stageMask =
          PipelineStages::FragmentShader | PipelineStages::ComputeShader,
        .accessMask = Access::ShaderRead,
      });
  });
}

} // namespace rhi
