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
VkDescriptorSetLayout
PipelineLayout::getDescriptorSet(const DescriptorSetIndex index) const {
  assert(index < m_descriptorSetLayouts.size());
  return m_descriptorSetLayouts[index];
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

Builder &Builder::addImage(const DescriptorSetIndex setIndex,
                           const BindingIndex bindingIndex,
                           const VkShaderStageFlags stages) {
  return addImages(setIndex, bindingIndex, 1, stages);
}
Builder &Builder::addImages(const DescriptorSetIndex setIndex,
                            const BindingIndex bindingIndex,
                            const uint32_t count,
                            const VkShaderStageFlags stages) {
  return addResource(setIndex,
                     {
                       .binding = bindingIndex,
                       .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                       .descriptorCount = count,
                       .stageFlags = stages,
                     });
}
Builder &Builder::addSampledImage(const DescriptorSetIndex setIndex,
                                  const BindingIndex bindingIndex,
                                  const VkShaderStageFlags stages) {
  return addSampledImages(setIndex, bindingIndex, 1, stages);
}
Builder &Builder::addSampledImages(const DescriptorSetIndex setIndex,
                                   const BindingIndex bindingIndex,
                                   const uint32_t count,
                                   const VkShaderStageFlags stages) {
  return addResource(
    setIndex, {
                .binding = bindingIndex,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = count,
                .stageFlags = stages,
              });
}
Builder &Builder::addUniformBuffer(const DescriptorSetIndex setIndex,
                                   const BindingIndex bindingIndex,
                                   const VkShaderStageFlags stages) {
  return addResource(setIndex,
                     {
                       .binding = bindingIndex,
                       .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       .descriptorCount = 1,
                       .stageFlags = stages,
                     });
}
Builder &Builder::addStorageBuffer(const DescriptorSetIndex setIndex,
                                   const BindingIndex bindingIndex,
                                   const VkShaderStageFlags stages) {
  return addResource(setIndex,
                     {
                       .binding = bindingIndex,
                       .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       .descriptorCount = 1,
                       .stageFlags = stages,
                     });
}

Builder &Builder::addResource(const DescriptorSetIndex index,
                              VkDescriptorSetLayoutBinding desc) {
  assert(index <= m_layoutInfo.descriptorSets.size());
  m_layoutInfo.descriptorSets[index].emplace_back(std::move(desc));
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
       reflection.descriptorSets | std::views::enumerate) {
    for (const auto &[index, resource] : bindings) {
      builder.addResource(static_cast<DescriptorSetIndex>(set),
                          {
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
