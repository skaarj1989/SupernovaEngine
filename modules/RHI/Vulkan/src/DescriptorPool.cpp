#include "rhi/DescriptorPool.hpp"
#include "VkCheck.hpp"
#include <array>

namespace rhi {

DescriptorPool::DescriptorPool(DescriptorPool &&other) noexcept
    : m_device{other.m_device}, m_descriptorPool{other.m_descriptorPool} {
  other.m_device = VK_NULL_HANDLE;
  other.m_descriptorPool = VK_NULL_HANDLE;
}
DescriptorPool::~DescriptorPool() { _destroy(); }

DescriptorPool &DescriptorPool::operator=(DescriptorPool &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_device, rhs.m_device);
    std::swap(m_descriptorPool, rhs.m_descriptorPool);
  }
  return *this;
}

VkDescriptorSet
DescriptorPool::allocate(VkDescriptorSetLayout descriptorSetLayout) {
  assert(descriptorSetLayout != VK_NULL_HANDLE);

  VkDescriptorSetAllocateInfo allocateInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = m_descriptorPool,
    .descriptorSetCount = 1,
    .pSetLayouts = &descriptorSetLayout,
  };
  VkDescriptorSet handle{VK_NULL_HANDLE};
  VK_CHECK(vkAllocateDescriptorSets(m_device, &allocateInfo, &handle));
  return handle;
}

void DescriptorPool::reset() {
  assert(m_descriptorPool != VK_NULL_HANDLE);
  VK_CHECK(vkResetDescriptorPool(m_device, m_descriptorPool,
                                 0 /* Must be 0, reserved for future use. */));
}

//
// (private):
//

DescriptorPool::DescriptorPool(VkDevice device) : m_device{device} {
  assert(device != VK_NULL_HANDLE);

#define POOL_SIZE(Type, Count)                                                 \
  VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_##Type, Count }

  // clang-format off
  const auto poolSizes = std::array{
    POOL_SIZE(SAMPLER, 1000),
    POOL_SIZE(COMBINED_IMAGE_SAMPLER, 1000),
    POOL_SIZE(SAMPLED_IMAGE, 1000),
    POOL_SIZE(STORAGE_IMAGE, 1000),
    POOL_SIZE(UNIFORM_BUFFER, 1000),
    POOL_SIZE(STORAGE_BUFFER, 1000),
  };
  // clang-format on
#undef POOL_SIZE

  const VkDescriptorPoolCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = 0,
    .maxSets = 1000,
    .poolSizeCount = poolSizes.size(),
    .pPoolSizes = poolSizes.data(),
  };
  VK_CHECK(
    vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorPool));
}

void DescriptorPool::_destroy() noexcept {
  if (m_descriptorPool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

    m_device = VK_NULL_HANDLE;
    m_descriptorPool = VK_NULL_HANDLE;
  }
}

} // namespace rhi
