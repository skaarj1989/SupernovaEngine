#pragma once

#include "glad/vulkan.h"

namespace rhi {

class CommandBuffer;

class DescriptorPool final {
  friend class CommandBuffer; // Calls the private constructor.

public:
  DescriptorPool() = default;
  DescriptorPool(const DescriptorPool &) = delete;
  DescriptorPool(DescriptorPool &&) noexcept;
  ~DescriptorPool();

  DescriptorPool &operator=(const DescriptorPool &) = delete;
  DescriptorPool &operator=(DescriptorPool &&) noexcept;

  [[nodiscard]] VkDescriptorSet allocate(VkDescriptorSetLayout);
  void reset();

private:
  explicit DescriptorPool(VkDevice);

  void _destroy() noexcept;

private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
};

} // namespace rhi
