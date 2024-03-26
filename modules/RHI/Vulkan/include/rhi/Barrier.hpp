#pragma once

#include "BarrierScope.hpp"
#include "ImageLayout.hpp"
#include <vector>

namespace rhi {

class CommandBuffer;
class Buffer;
class Texture;

class Barrier final {
  friend class CommandBuffer; // Reads m_info (in flushBarriers).

  struct Dependencies {
    Dependencies() {
      buffer.reserve(10);
      image.reserve(10);
    }

    std::vector<VkMemoryBarrier2> memory;
    std::vector<VkBufferMemoryBarrier2> buffer;
    std::vector<VkImageMemoryBarrier2> image;
  };

public:
  [[nodiscard]] bool isEffective() const;

  class Builder {
    friend class CommandBuffer; // Calls _imageBarrier (in generateMipmaps).

  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = default;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = default;

    Builder &memoryBarrier(const BarrierScope &src, const BarrierScope &dst);

    struct BufferInfo {
      const Buffer &buffer;
      VkDeviceSize offset{0};
      VkDeviceSize size{VK_WHOLE_SIZE};
    };
    Builder &bufferBarrier(const BufferInfo &, const BarrierScope &dst);

    struct ImageInfo {
      const Texture &image;
      ImageLayout newLayout{ImageLayout::Undefined};
      VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_NONE,
        .baseMipLevel = 0,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0,
        .layerCount = VK_REMAINING_ARRAY_LAYERS,
      };
    };
    Builder &imageBarrier(ImageInfo, const BarrierScope &dst);

    [[nodiscard]] Barrier build();

  private:
    Builder &_imageBarrier(const VkImage, const BarrierScope &src,
                           const BarrierScope &dst, const ImageLayout oldLayout,
                           const ImageLayout newLayout,
                           const VkImageSubresourceRange &);

  private:
    Dependencies m_dependencies;
  };

private:
  explicit Barrier(Dependencies &&);

private:
  VkDependencyInfo m_info{};
  Dependencies m_dependencies;
};

} // namespace rhi
