#pragma once

#include "BarrierScope.hpp"
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

  using Stride = uint32_t;

  [[nodiscard]] VkBuffer getHandle() const;
  [[nodiscard]] VkDeviceSize getSize() const;

  void *map();
  Buffer &unmap();

  Buffer &flush(const VkDeviceSize offset = 0,
                const VkDeviceSize size = VK_WHOLE_SIZE);

private:
  Buffer(const VmaAllocator, const VkDeviceSize size, const VkBufferUsageFlags,
         const VmaAllocationCreateFlags, const VmaMemoryUsage);

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
