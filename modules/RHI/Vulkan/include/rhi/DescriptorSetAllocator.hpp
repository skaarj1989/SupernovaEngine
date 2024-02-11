#pragma once

#include "glad/vulkan.h"
#include <vector>

namespace rhi {

class CommandBuffer;

struct DescriptorPool {
  VkDescriptorPool handle{VK_NULL_HANDLE};
  uint32_t numAllocatedSets{0};

  static const uint32_t kSetsPerPool;
};

class DescriptorSetAllocator final {
  friend class CommandBuffer; // Calls the private constructor.

public:
  DescriptorSetAllocator() = default;
  DescriptorSetAllocator(const DescriptorSetAllocator &) = delete;
  DescriptorSetAllocator(DescriptorSetAllocator &&) noexcept;
  ~DescriptorSetAllocator();

  DescriptorSetAllocator &operator=(const DescriptorSetAllocator &) = delete;
  DescriptorSetAllocator &operator=(DescriptorSetAllocator &&) noexcept;

  [[nodiscard]] VkDescriptorSet allocate(VkDescriptorSetLayout);
  void reset();

private:
  explicit DescriptorSetAllocator(VkDevice);

  void _destroy() noexcept;

  [[nodiscard]] DescriptorPool &_createPool();
  [[nodiscard]] DescriptorPool &_getPool();
  [[nodiscard]] VkDescriptorSet _allocate(DescriptorPool &,
                                          VkDescriptorSetLayout) const;

private:
  VkDevice m_device{VK_NULL_HANDLE};

  std::vector<DescriptorPool> m_descriptorPools;
  int32_t m_lastPoolIndex{-1};
};

} // namespace rhi
