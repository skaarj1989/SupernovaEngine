#include "FrameGraphImport.hpp"
#include "fg/FrameGraph.hpp"
#include "rhi/Texture.hpp"
#include "renderer/FrameGraphTexture.hpp"

namespace gfx {

FrameGraphResource importTexture(FrameGraph &fg, const std::string_view name,
                                 rhi::Texture *texture) {
  assert(texture && *texture);
  return fg.import <FrameGraphTexture>(
    name,
    {
      .extent = texture->getExtent(),
      .depth = texture->getDepth(),
      .format = texture->getPixelFormat(),
      .numMipLevels = texture->getNumMipLevels(),
      .layers = texture->getNumLayers(),
      .cubemap = rhi::isCubemap(*texture),
      .usageFlags = texture->getUsageFlags(),
    },
    {texture});
}

} // namespace gfx
