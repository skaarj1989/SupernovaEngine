#pragma once

#include "glad/vulkan.h"
#include "ResourceIndices.hpp"
#include <array>
#include <vector>

namespace rhi {

constexpr auto kMinNumDescriptorSets = 4;

struct PipelineLayoutInfo {
  using DescriptorSetBindings = std::vector<VkDescriptorSetLayoutBinding>;
  std::array<DescriptorSetBindings, kMinNumDescriptorSets> descriptorSets;
  std::vector<VkPushConstantRange> pushConstantRanges;
};

class RenderDevice;

class PipelineLayout final {
  friend class RenderDevice; // Calls the private constructor.

public:
  PipelineLayout() = default;
  PipelineLayout(const PipelineLayout &) = default;
  PipelineLayout(PipelineLayout &&) noexcept;
  ~PipelineLayout() = default;

  PipelineLayout &operator=(const PipelineLayout &) = default;
  PipelineLayout &operator=(PipelineLayout &&) noexcept;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] VkPipelineLayout getHandle() const;
  [[nodiscard]] VkDescriptorSetLayout
  getDescriptorSet(const DescriptorSetIndex) const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &addImage(const DescriptorSetIndex, const BindingIndex,
                      const VkShaderStageFlags);
    Builder &addImages(const DescriptorSetIndex, const BindingIndex,
                       const uint32_t count, const VkShaderStageFlags);
    Builder &addSampledImage(const DescriptorSetIndex, const BindingIndex,
                             const VkShaderStageFlags);
    Builder &addSampledImages(const DescriptorSetIndex, const BindingIndex,
                              const uint32_t count, const VkShaderStageFlags);
    Builder &addUniformBuffer(const DescriptorSetIndex, const BindingIndex,
                              const VkShaderStageFlags);
    Builder &addStorageBuffer(const DescriptorSetIndex, const BindingIndex,
                              const VkShaderStageFlags);

    Builder &addResource(const DescriptorSetIndex,
                         VkDescriptorSetLayoutBinding);
    Builder &addPushConstantRange(VkPushConstantRange);

    [[nodiscard]] PipelineLayout build(RenderDevice &) const;

  private:
    PipelineLayoutInfo m_layoutInfo;
  };

private:
  PipelineLayout(const VkPipelineLayout, std::vector<VkDescriptorSetLayout> &&);

private:
  VkPipelineLayout m_handle{VK_NULL_HANDLE}; // Non-owning.
  std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};

struct ShaderReflection;

[[nodiscard]] PipelineLayout reflectPipelineLayout(RenderDevice &,
                                                   const ShaderReflection &);

} // namespace rhi
