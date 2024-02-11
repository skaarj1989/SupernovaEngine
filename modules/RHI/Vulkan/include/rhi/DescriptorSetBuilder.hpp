#pragma once

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
  std::optional<VkSampler> sampler;
};
struct SampledImage {
  const Texture *texture{nullptr};
};
struct StorageImage {
  const Texture *texture{nullptr};
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
  DescriptorSetBuilder(VkDevice, DescriptorSetAllocator &,
                       DescriptorSetCache &);
  DescriptorSetBuilder(const DescriptorSetBuilder &) = delete;
  DescriptorSetBuilder(DescriptorSetBuilder &&) noexcept = delete;

  DescriptorSetBuilder &operator=(const DescriptorSetBuilder &) = delete;
  DescriptorSetBuilder &operator=(DescriptorSetBuilder &&) noexcept = delete;

  DescriptorSetBuilder &bind(uint32_t index, const ResourceBinding &);
  DescriptorSetBuilder &bind(uint32_t index, const bindings::SeparateSampler &);
  DescriptorSetBuilder &bind(uint32_t index,
                             const bindings::CombinedImageSampler &);
  DescriptorSetBuilder &bind(uint32_t index, const bindings::SampledImage &);
  DescriptorSetBuilder &bind(uint32_t index, const bindings::StorageImage &);
  DescriptorSetBuilder &bind(uint32_t index, const bindings::UniformBuffer &);
  DescriptorSetBuilder &bind(uint32_t index, const bindings::StorageBuffer &);

  [[nodiscard]] VkDescriptorSet build(VkDescriptorSetLayout);

private:
  void _clear();

  void _addImage(VkImageView, VkImageLayout);
  void _addSampler(VkSampler);
  void _addCombinedImageSampler(VkImageView, VkImageLayout, VkSampler);

  DescriptorSetBuilder &_bindBuffer(uint32_t index, VkDescriptorType,
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
  // Key = binding
  // layout(binding = index)
  robin_hood::unordered_map<uint32_t, BindingInfo> m_bindings;
  union DescriptorVariant {
    VkDescriptorImageInfo imageInfo;
    VkDescriptorBufferInfo bufferInfo;
  };
  std::vector<DescriptorVariant> m_descriptors;
};

[[nodiscard]] const char *toString(const ResourceBinding &);

} // namespace rhi
