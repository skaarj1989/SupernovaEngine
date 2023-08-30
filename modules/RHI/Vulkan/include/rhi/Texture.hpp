#pragma once

#include "rhi/Extent2D.hpp"
#include "rhi/TextureType.hpp"
#include "rhi/CubeFace.hpp"
#include "rhi/PixelFormat.hpp"
#include "rhi/ImageLayout.hpp"
#include "rhi/BarrierScope.hpp"
#include "glm/ext/vector_uint3.hpp"
#include "vk_mem_alloc.h"

#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <span>

namespace rhi {

enum class ImageUsage {
  TransferSrc = 1 << 0,
  TransferDst = 1 << 1,
  Transfer = TransferSrc | TransferDst,
  Storage = 1 << 2,
  RenderTarget = 1 << 3,
  Sampled = 1 << 4,
};
template <> struct has_flags<ImageUsage> : std::true_type {};

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

  void setSampler(VkSampler);

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

  [[nodiscard]] VkImageView getImageView() const;

  [[nodiscard]] VkImageView getMipLevel(uint32_t) const;
  [[nodiscard]] std::span<const VkImageView> getMipLevels() const;
  [[nodiscard]] std::span<const VkImageView> getLayers() const;
  [[nodiscard]] VkImageView getLayer(uint32_t, std::optional<CubeFace>) const;

  [[nodiscard]] VkSampler getSampler() const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setExtent(rhi::Extent2D, uint32_t depth = 0);
    Builder &setPixelFormat(PixelFormat);
    Builder &setNumMipLevels(std::optional<uint32_t>);
    Builder &setNumLayers(std::optional<uint32_t>);
    Builder &setCubemap(bool);
    Builder &setUsageFlags(ImageUsage);
    Builder &setupOptimalSampler(bool);

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
  Texture(VmaAllocator, CreateInfo &&);
  // "Import" image (from a Swapchain).
  Texture(VkDevice, VkImage, Extent2D, PixelFormat);

  void _destroy() noexcept;

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

  VkImageView m_imageView{VK_NULL_HANDLE};
  std::vector<VkImageView> m_mipLevels;
  std::vector<VkImageView> m_layers;
  VkSampler m_sampler{VK_NULL_HANDLE}; // Non-owning.

  Extent2D m_extent{0u};
  uint32_t m_depth{0u};
  PixelFormat m_format{PixelFormat::Undefined};
  uint32_t m_numMipLevels{1u};
  uint32_t m_numLayers{0u};  // 0 = Non-layered.
  uint32_t m_layerFaces{0u}; // Internal use.
  ImageUsage m_usageFlags{ImageUsage::Sampled};
};

[[nodiscard]] bool isFormatSupported(const RenderDevice &, PixelFormat,
                                     ImageUsage);
[[nodiscard]] VkImageAspectFlags getAspectMask(const Texture &);

[[nodiscard]] uint32_t calcMipLevels(Extent2D);
[[nodiscard]] uint32_t calcMipLevels(uint32_t size);
[[nodiscard]] glm::uvec3 calcMipSize(const glm::uvec3 &baseSize,
                                     uint32_t level);

[[nodiscard]] bool isCubemap(const Texture &);

[[nodiscard]] std::string toString(ImageUsage);

} // namespace rhi
