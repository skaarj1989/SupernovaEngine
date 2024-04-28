#pragma once

#include "ImageAspect.hpp"
#include "ResourceIndices.hpp"
#include "glad/vulkan.h"
#include "robin_hood.h"
#include <variant>
#include <optional>

namespace rhi {

class DescriptorSetAllocator;
class Buffer;
class Texture;

// Key = Hash.
using DescriptorSetCache =
  robin_hood::unordered_map<std::size_t, VkDescriptorSet>;

namespace bindings {

struct SeparateSampler {
  VkSampler handle{VK_NULL_HANDLE};
};
struct CombinedImageSampler {
  const Texture *texture{nullptr};
  rhi::ImageAspect imageAspect{rhi::ImageAspect::None};
  std::optional<VkSampler> sampler;
};
struct SampledImage {
  const Texture *texture{nullptr};
  rhi::ImageAspect imageAspect{rhi::ImageAspect::None};
};
struct StorageImage {
  const Texture *texture{nullptr};
  rhi::ImageAspect imageAspect{rhi::ImageAspect::None};
  std::optional<uint32_t> mipLevel;
};

struct UniformBuffer {
  const Buffer *buffer{nullptr};
  VkDeviceSize offset{0};
  std::optional<VkDeviceSize> range;
};
struct StorageBuffer {
  const Buffer *buffer{nullptr};
  VkDeviceSize offset{0};
  std::optional<VkDeviceSize> range;
};

} // namespace bindings

// clang-format off
using ResourceBinding = std::variant<
  bindings::SeparateSampler,
  bindings::CombinedImageSampler,
  bindings::SampledImage,
  bindings::StorageImage,
  bindings::UniformBuffer,
  bindings::StorageBuffer
>;
// clang-format on

class DescriptorSetBuilder final {
public:
  DescriptorSetBuilder() = delete;
  DescriptorSetBuilder(const VkDevice, DescriptorSetAllocator &,
                       DescriptorSetCache &);
  DescriptorSetBuilder(const DescriptorSetBuilder &) = delete;
  DescriptorSetBuilder(DescriptorSetBuilder &&) noexcept = delete;

  DescriptorSetBuilder &operator=(const DescriptorSetBuilder &) = delete;
  DescriptorSetBuilder &operator=(DescriptorSetBuilder &&) noexcept = delete;

  DescriptorSetBuilder &bind(const BindingIndex, const ResourceBinding &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::SeparateSampler &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::CombinedImageSampler &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::SampledImage &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::StorageImage &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::UniformBuffer &);
  DescriptorSetBuilder &bind(const BindingIndex,
                             const bindings::StorageBuffer &);

  [[nodiscard]] VkDescriptorSet build(const VkDescriptorSetLayout);

private:
  void _clear();

  void _addImage(const VkImageView, const VkImageLayout);
  void _addSampler(const VkSampler);
  void _addCombinedImageSampler(const VkImageView, const VkImageLayout,
                                const VkSampler);

  DescriptorSetBuilder &_bindBuffer(const BindingIndex, const VkDescriptorType,
                                    VkDescriptorBufferInfo &&);

private:
  VkDevice m_device{VK_NULL_HANDLE};
  DescriptorSetAllocator &m_descriptorSetAllocator;
  DescriptorSetCache &m_descriptorSetCache;

  struct BindingInfo {
    VkDescriptorType type;
    uint32_t count{0};
    int32_t descriptorId{-1}; // Index to m_descriptors.
  };
  // layout(binding = index)
  robin_hood::unordered_map<BindingIndex, BindingInfo> m_bindings;
  union DescriptorVariant {
    VkDescriptorImageInfo imageInfo;
    VkDescriptorBufferInfo bufferInfo;
  };
  std::vector<DescriptorVariant> m_descriptors;
};

[[nodiscard]] const char *toString(const ResourceBinding &);

} // namespace rhi
