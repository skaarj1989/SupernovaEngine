#include "rhi/PipelineLayout.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderReflection.hpp"

#include <ranges>

namespace rhi {

namespace {

template <typename T>
using DescriptorContainerSize = std::tuple_size<decltype(T::descriptorSets)>;

static_assert(DescriptorContainerSize<PipelineLayoutInfo>::value ==
              DescriptorContainerSize<ShaderReflection>::value);

} // namespace

//
// PipelineLayout class:
//

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept
    : m_handle{other.m_handle},
      m_descriptorSetLayouts{std::move(other.m_descriptorSetLayouts)} {
  other.m_handle = VK_NULL_HANDLE;
}

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&rhs) noexcept {
  if (this != &rhs) {
    m_handle = std::exchange(rhs.m_handle, VK_NULL_HANDLE);
    m_descriptorSetLayouts = std::move(rhs.m_descriptorSetLayouts);
  }
  return *this;
}

PipelineLayout::operator bool() const { return m_handle != VK_NULL_HANDLE; }

VkPipelineLayout PipelineLayout::getHandle() const { return m_handle; }
VkDescriptorSetLayout PipelineLayout::getDescriptorSet(uint32_t set) const {
  assert(set < m_descriptorSetLayouts.size());
  return m_descriptorSetLayouts[set];
}

PipelineLayout::PipelineLayout(
  VkPipelineLayout handle,
  std::vector<VkDescriptorSetLayout> &&descriptorSetLayouts)
    : m_handle{handle},
      m_descriptorSetLayouts{std::move(descriptorSetLayouts)} {}

//
// Builder class:
//

using Builder = PipelineLayout::Builder;

Builder &Builder::addImage(uint32_t set, uint32_t binding,
                           VkShaderStageFlags stages) {
  return addImages(set, binding, 1, stages);
}
Builder &Builder::addImages(uint32_t set, uint32_t binding, uint32_t count,
                            VkShaderStageFlags stages) {
  return addResource(set, {
                            .binding = binding,
                            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                            .descriptorCount = count,
                            .stageFlags = stages,
                          });
}
Builder &Builder::addSampledImage(uint32_t set, uint32_t binding,
                                  VkShaderStageFlags stages) {
  return addSampledImages(set, binding, 1, stages);
}
Builder &Builder::addSampledImages(uint32_t set, uint32_t binding,
                                   uint32_t count, VkShaderStageFlags stages) {
  return addResource(
    set, {
           .binding = binding,
           .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
           .descriptorCount = count,
           .stageFlags = stages,
         });
}
Builder &Builder::addUniformBuffer(uint32_t set, uint32_t binding,
                                   VkShaderStageFlags stages) {
  return addResource(set, {
                            .binding = binding,
                            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = 1,
                            .stageFlags = stages,
                          });
}
Builder &Builder::addStorageBuffer(uint32_t set, uint32_t binding,
                                   VkShaderStageFlags stages) {
  return addResource(set, {
                            .binding = binding,
                            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                            .descriptorCount = 1,
                            .stageFlags = stages,
                          });
}

Builder &Builder::addResource(uint32_t set, VkDescriptorSetLayoutBinding desc) {
  assert(set <= m_layoutInfo.descriptorSets.size());
  m_layoutInfo.descriptorSets[set].emplace_back(std::move(desc));
  return *this;
}

Builder &Builder::addPushConstantRange(VkPushConstantRange pushConstantRange) {
  assert(pushConstantRange.stageFlags != VK_PIPELINE_STAGE_2_NONE);
  m_layoutInfo.pushConstantRanges.emplace_back(std::move(pushConstantRange));
  return *this;
}

PipelineLayout Builder::build(RenderDevice &rd) const {
  return rd.createPipelineLayout(m_layoutInfo);
}

//
// Utility:
//

PipelineLayout reflectPipelineLayout(RenderDevice &rd,
                                     const ShaderReflection &reflection) {
  PipelineLayout::Builder builder{};

  for (auto [set, bindings] :
       std::views::enumerate(reflection.descriptorSets)) {
    for (const auto &[index, resource] : reflection.descriptorSets[set]) {
      builder.addResource(set, {
                                 .binding = index,
                                 .descriptorType = resource.type,
                                 .descriptorCount = resource.count,
                                 .stageFlags = resource.stageFlags,
                               });
    }
  }
  for (const auto &range : reflection.pushConstantRanges) {
    builder.addPushConstantRange(range);
  }
  return builder.build(rd);
}

} // namespace rhi
