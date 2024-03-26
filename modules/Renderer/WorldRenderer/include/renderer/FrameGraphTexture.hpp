#pragma once

#include "rhi/Extent2D.hpp"
#include "rhi/PixelFormat.hpp"
#include "rhi/ImageUsage.hpp"

namespace rhi {
class Texture;
}

namespace gfx {

class FrameGraphTexture {
public:
  struct Desc {
    rhi::Extent2D extent;
    uint32_t depth{0};
    rhi::PixelFormat format{rhi::PixelFormat::Undefined};
    uint32_t numMipLevels{1};
    uint32_t layers{0};
    bool cubemap{false};
    rhi::ImageUsage usageFlags;
  };

  void create(const Desc &, void *allocator);
  void destroy(const Desc &, void *allocator);

  void preRead(const Desc &, uint32_t flags, void *context);
  void preWrite(const Desc &, uint32_t flags, void *context);

  [[nodiscard]] static std::string toString(const Desc &);

  rhi::Texture *texture{nullptr};
};

} // namespace gfx
