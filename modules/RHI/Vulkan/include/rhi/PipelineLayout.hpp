#pragma once

#include "glad/vulkan.h"
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
  [[nodiscard]] VkDescriptorSetLayout getDescriptorSet(uint32_t set) const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &addImage(uint32_t set, uint32_t binding, VkShaderStageFlags);
    Builder &addImages(uint32_t set, uint32_t binding, uint32_t count,
                       VkShaderStageFlags);
    Builder &addSampledImage(uint32_t set, uint32_t binding,
                             VkShaderStageFlags);
    Builder &addSampledImages(uint32_t set, uint32_t binding, uint32_t count,
                              VkShaderStageFlags);
    Builder &addUniformBuffer(uint32_t set, uint32_t binding,
                              VkShaderStageFlags);
    Builder &addStorageBuffer(uint32_t set, uint32_t binding,
                              VkShaderStageFlags);

    Builder &addResource(uint32_t set, VkDescriptorSetLayoutBinding);
    Builder &addPushConstantRange(VkPushConstantRange);

    [[nodiscard]] PipelineLayout build(RenderDevice &) const;

  private:
    PipelineLayoutInfo m_layoutInfo;
  };

private:
  PipelineLayout(VkPipelineLayout, std::vector<VkDescriptorSetLayout> &&);

private:
  VkPipelineLayout m_handle{VK_NULL_HANDLE}; // Non-owning.
  std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};

struct ShaderReflection;

[[nodiscard]] PipelineLayout reflectPipelineLayout(RenderDevice &,
                                                   const ShaderReflection &);

} // namespace rhi
