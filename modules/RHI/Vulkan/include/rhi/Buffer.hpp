#pragma once

#include "rhi/BarrierScope.hpp"
#include "vk_mem_alloc.h"

namespace rhi {

class RenderDevice;
class Barrier;

class Buffer {
  friend class RenderDevice; // Calls the private constructor.
  friend class Barrier;      // Modifies m_lastScope.

public:
  Buffer() = default;
  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&) noexcept;
  virtual ~Buffer();

  Buffer &operator=(const Buffer &) = delete;
  Buffer &operator=(Buffer &&) noexcept;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] VkBuffer getHandle() const;
  [[nodiscard]] VkDeviceSize getSize() const;

  void *map();
  Buffer &unmap();

  Buffer &flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

private:
  Buffer(VmaAllocator, VkDeviceSize size, VkBufferUsageFlags,
         VmaAllocationCreateFlags, VmaMemoryUsage);

  void _destroy() noexcept;

private:
  VmaAllocator m_memoryAllocator{VK_NULL_HANDLE};
  VmaAllocation m_allocation{VK_NULL_HANDLE};
  VkBuffer m_handle{VK_NULL_HANDLE};
  mutable BarrierScope m_lastScope{kInitialBarrierScope};

  VkDeviceSize m_size{0};
  void *m_mappedMemory{nullptr};
};

} // namespace rhi
