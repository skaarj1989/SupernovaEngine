#pragma once

#include "Extent2D.hpp"
#include "ImageUsage.hpp"
#include "TextureType.hpp"
#include "CubeFace.hpp"
#include "PixelFormat.hpp"
#include "ImageAspect.hpp"
#include "ImageLayout.hpp"
#include "BarrierScope.hpp"

#include "glm/ext/vector_uint3.hpp"
#include "vk_mem_alloc.h"

#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <span>

namespace rhi {

class RenderDevice;
class Swapchain;
class CommandBuffer;
class Barrier;

class Texture {
  friend class RenderDevice;  // Calls the 1st private constructor.
  friend class Swapchain;     // Calls the 2nd private constructor.
  friend class CommandBuffer; // Layout transition (generateMipmaps).
  friend class Barrier;       // Modifies m_lastScope.

public:
  Texture() = default;
  Texture(const Texture &) = delete;
  Texture(Texture &&) noexcept;
  virtual ~Texture();

  Texture &operator=(const Texture &) = delete;
  Texture &operator=(Texture &&) noexcept;

  [[nodiscard]] bool operator==(const Texture &) const;

  [[nodiscard]] explicit operator bool() const;

  // ---

  void setSampler(const VkSampler);

  // ---

  [[nodiscard]] TextureType getType() const;
  [[nodiscard]] Extent2D getExtent() const;
  [[nodiscard]] uint32_t getDepth() const;
  [[nodiscard]] uint32_t getNumMipLevels() const;
  [[nodiscard]] uint32_t getNumLayers() const;
  [[nodiscard]] PixelFormat getPixelFormat() const;
  [[nodiscard]] ImageUsage getUsageFlags() const;

  [[nodiscard]] VkImage getImageHandle() const;
  [[nodiscard]] ImageLayout getImageLayout() const;

  // @return Used memory (in bytes).
  [[nodiscard]] VkDeviceSize getSize() const;

  [[nodiscard]] VkImageView
  getImageView(const VkImageAspectFlags = VK_IMAGE_ASPECT_NONE) const;

  [[nodiscard]] VkImageView
  getMipLevel(const uint32_t,
              const VkImageAspectFlags = VK_IMAGE_ASPECT_NONE) const;
  [[nodiscard]] std::span<const VkImageView>
  getMipLevels(const VkImageAspectFlags = VK_IMAGE_ASPECT_NONE) const;
  [[nodiscard]] VkImageView
  getLayer(const uint32_t, const std::optional<CubeFace>,
           const VkImageAspectFlags = VK_IMAGE_ASPECT_NONE) const;
  [[nodiscard]] std::span<const VkImageView>
  getLayers(const VkImageAspectFlags = VK_IMAGE_ASPECT_NONE) const;

  [[nodiscard]] VkSampler getSampler() const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setExtent(const Extent2D, const uint32_t depth = 0);
    Builder &setPixelFormat(const PixelFormat);
    Builder &setNumMipLevels(const std::optional<uint32_t>);
    Builder &setNumLayers(const std::optional<uint32_t>);
    Builder &setCubemap(const bool);
    Builder &setUsageFlags(const ImageUsage);
    Builder &setupOptimalSampler(const bool);

    [[nodiscard]] Texture build(RenderDevice &);

  private:
    Extent2D m_extent{};
    uint32_t m_depth{0};
    PixelFormat m_pixelFormat{PixelFormat::Undefined};
    std::optional<uint32_t> m_numMipLevels{};
    std::optional<uint32_t> m_numLayers{};
    bool m_isCubemap{false};
    ImageUsage m_usageFlags{0};

    bool m_setupOptimalSampler{false};
  };

private:
  struct CreateInfo {
    Extent2D extent;
    uint32_t depth{0};
    PixelFormat pixelFormat{PixelFormat::Undefined};
    uint32_t numMipLevels{1u};
    uint32_t numLayers{0u};
    uint32_t numFaces{1u};
    ImageUsage usageFlags{ImageUsage::Sampled};
  };
  Texture(const VmaAllocator, CreateInfo &&);
  // "Import" image (from a Swapchain).
  Texture(const VkDevice, const VkImage, const Extent2D, const PixelFormat);

  void _destroy() noexcept;

  VkDevice _getDeviceHandle() const;

  struct AspectData {
    VkImageView imageView{VK_NULL_HANDLE};
    std::vector<VkImageView> mipLevels;
    std::vector<VkImageView> layers;
  };
  void _createAspect(const VkDevice, const VkImage, const VkImageViewType,
                     const VkImageAspectFlags, AspectData &);
  const AspectData *_getAspect(const VkImageAspectFlags) const;

private:
  using DeviceOrAllcator = std::variant<std::monostate, VkDevice, VmaAllocator>;
  DeviceOrAllcator m_deviceOrAllocator{};

  struct AllocatedImage {
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkImage handle{VK_NULL_HANDLE};

    auto operator<=>(const AllocatedImage &) const = default;
  };
  using ImageVariant = std::variant<std::monostate, VkImage, AllocatedImage>;
  ImageVariant m_image{};

  TextureType m_type{TextureType::Undefined};

  mutable ImageLayout m_layout{ImageLayout::Undefined};
  mutable BarrierScope m_lastScope{kInitialBarrierScope};

  std::unordered_map<VkImageAspectFlags, AspectData> m_aspects;

  VkSampler m_sampler{VK_NULL_HANDLE}; // Non-owning.

  Extent2D m_extent{0u};
  uint32_t m_depth{0u};
  PixelFormat m_format{PixelFormat::Undefined};
  uint32_t m_numMipLevels{1u};
  uint32_t m_numLayers{0u};  // 0 = Non-layered.
  uint32_t m_layerFaces{0u}; // Internal use.
  ImageUsage m_usageFlags{ImageUsage::Sampled};
};

[[nodiscard]] bool isFormatSupported(const RenderDevice &, const PixelFormat,
                                     const ImageUsage);
[[nodiscard]] VkImageAspectFlags getAspectMask(const Texture &);

[[nodiscard]] uint32_t calcMipLevels(const Extent2D);
[[nodiscard]] uint32_t calcMipLevels(const uint32_t size);
[[nodiscard]] glm::uvec3 calcMipSize(const glm::uvec3 &baseSize,
                                     const uint32_t level);

[[nodiscard]] bool isCubemap(const Texture &);

} // namespace rhi
