#include "rhi/DescriptorSetAllocator.hpp"
#include "VkCheck.hpp"
#include <array>

namespace rhi {

const uint32_t DescriptorPool::kSetsPerPool = 100u;

namespace {

[[nodiscard]] auto createDescriptorPool(const VkDevice device) {
#define POOL_SIZE(Type, Multiplier)                                            \
  VkDescriptorPoolSize {                                                       \
    VK_DESCRIPTOR_TYPE_##Type,                                                 \
      static_cast<uint32_t>(DescriptorPool::kSetsPerPool * Multiplier)         \
  }
  // clang-format off
  constexpr auto kPoolSizes = std::array{
    POOL_SIZE(SAMPLER, 0.26f),
    POOL_SIZE(COMBINED_IMAGE_SAMPLER, 5.4f),
    POOL_SIZE(SAMPLED_IMAGE, 1.81f),
    POOL_SIZE(STORAGE_IMAGE, 0.12f),
    POOL_SIZE(UNIFORM_BUFFER, 2.2f),
    POOL_SIZE(STORAGE_BUFFER, 3.6f),
  };
  // clang-format on
#undef POOL_SIZE

  const VkDescriptorPoolCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
    .flags = 0,
    .maxSets = DescriptorPool::kSetsPerPool,
    .poolSizeCount = kPoolSizes.size(),
    .pPoolSizes = kPoolSizes.data(),
  };
  VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
  VK_CHECK(
    vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));
  return descriptorPool;
}

} // namespace

//
// DescriptorSetAllocator class:
//

DescriptorSetAllocator::DescriptorSetAllocator(
  DescriptorSetAllocator &&other) noexcept
    : m_device{other.m_device}, m_descriptorPools{other.m_descriptorPools},
      m_lastPoolIndex{other.m_lastPoolIndex} {
  other.m_device = VK_NULL_HANDLE;
  other.m_descriptorPools = {};
  other.m_lastPoolIndex = -1;
}
DescriptorSetAllocator::~DescriptorSetAllocator() { _destroy(); }

DescriptorSetAllocator &
DescriptorSetAllocator::operator=(DescriptorSetAllocator &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_device, rhs.m_device);
    std::swap(m_descriptorPools, rhs.m_descriptorPools);
    std::swap(m_lastPoolIndex, rhs.m_lastPoolIndex);
  }
  return *this;
}

VkDescriptorSet DescriptorSetAllocator::allocate(
  const VkDescriptorSetLayout descriptorSetLayout) {
  assert(m_device != VK_NULL_HANDLE && descriptorSetLayout != VK_NULL_HANDLE);
  auto descriptorSet = _allocate(_getPool(), descriptorSetLayout);
  if (descriptorSet == VK_NULL_HANDLE) {
    // No more space in the descriptor pool (any of .pPoolSizes)
    descriptorSet = _allocate(_createPool(), descriptorSetLayout);
  }
  assert(descriptorSet != VK_NULL_HANDLE);
  return descriptorSet;
}

void DescriptorSetAllocator::reset() {
  assert(m_device != VK_NULL_HANDLE);
  for (auto &[h, numAllocatedSets] : m_descriptorPools) {
    if (numAllocatedSets > 0) {
      VK_CHECK(vkResetDescriptorPool(
        m_device, h, 0 /* Must be 0, reserved for future use. */));
      numAllocatedSets = 0;
    }
  }
  m_lastPoolIndex = m_descriptorPools.empty() ? -1 : 0;
}

//
// (private):
//

DescriptorSetAllocator::DescriptorSetAllocator(const VkDevice device)
    : m_device{device} {
  assert(m_device != VK_NULL_HANDLE);
}

void DescriptorSetAllocator::_destroy() noexcept {
  if (m_device == VK_NULL_HANDLE) {
    assert(m_descriptorPools.empty());
    return;
  }
  for (const auto [h, _] : m_descriptorPools) {
    vkDestroyDescriptorPool(m_device, h, nullptr);
  }
  m_descriptorPools.clear();
  m_lastPoolIndex = -1;

  m_device = VK_NULL_HANDLE;
}

DescriptorPool &DescriptorSetAllocator::_createPool() {
  m_lastPoolIndex = m_descriptorPools.size();
  const auto descriptorPool = createDescriptorPool(m_device);
  return m_descriptorPools.emplace_back(descriptorPool);
}
DescriptorPool &DescriptorSetAllocator::_getPool() {
  // NOTE: Compiler will convert m_lastPoolIndex to size_t (-1 < 0u == false)
  for (; m_lastPoolIndex < m_descriptorPools.size(); ++m_lastPoolIndex) {
    auto &dp = m_descriptorPools[m_lastPoolIndex];
    if (dp.numAllocatedSets < DescriptorPool::kSetsPerPool) return dp;
  }
  return _createPool();
}
VkDescriptorSet DescriptorSetAllocator::_allocate(
  DescriptorPool &descriptorPool,
  const VkDescriptorSetLayout descriptorSetLayout) const {
  VkDescriptorSetAllocateInfo allocateInfo{
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    .descriptorPool = descriptorPool.handle,
    .descriptorSetCount = 1,
    .pSetLayouts = &descriptorSetLayout,
  };
  VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
  const auto result =
    vkAllocateDescriptorSets(m_device, &allocateInfo, &descriptorSet);
  switch (result) {
  case VK_SUCCESS:
  case VK_ERROR_OUT_OF_POOL_MEMORY:
  case VK_ERROR_FRAGMENTED_POOL:
    break;

  default:
    assert(false);
  }
  if (descriptorSet != VK_NULL_HANDLE) descriptorPool.numAllocatedSets++;
  return descriptorSet;
}

} // namespace rhi
