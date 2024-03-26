#include "rhi/Buffer.hpp"
#include "VkCheck.hpp"
#include <utility> // swap

namespace rhi {

Buffer::Buffer(Buffer &&other) noexcept
    : m_memoryAllocator{other.m_memoryAllocator},
      m_allocation{other.m_allocation}, m_handle{other.m_handle},
      m_lastScope{other.m_lastScope}, m_size{other.m_size},
      m_mappedMemory{other.m_mappedMemory} {
  other.m_memoryAllocator = VK_NULL_HANDLE;
  other.m_allocation = VK_NULL_HANDLE;
  other.m_handle = VK_NULL_HANDLE;
  other.m_lastScope = {};
  other.m_size = 0;
  other.m_mappedMemory = nullptr;
}
Buffer::~Buffer() { _destroy(); }

Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_memoryAllocator, rhs.m_memoryAllocator);
    std::swap(m_allocation, rhs.m_allocation);
    std::swap(m_handle, rhs.m_handle);
    std::swap(m_lastScope, rhs.m_lastScope);
    std::swap(m_size, rhs.m_size);
    std::swap(m_mappedMemory, rhs.m_mappedMemory);
  }
  return *this;
}

Buffer::operator bool() const { return m_handle != VK_NULL_HANDLE; }

VkBuffer Buffer::getHandle() const { return m_handle; }
VkDeviceSize Buffer::getSize() const { return m_size; }

void *Buffer::map() {
  assert(m_handle != VK_NULL_HANDLE);
  if (!m_mappedMemory) {
    VK_CHECK(vmaMapMemory(m_memoryAllocator, m_allocation, &m_mappedMemory));
  }

  return m_mappedMemory;
}
Buffer &Buffer::unmap() {
  assert(m_handle != VK_NULL_HANDLE);
  if (m_mappedMemory) {
    vmaUnmapMemory(m_memoryAllocator, m_allocation);
    m_mappedMemory = nullptr;
  }
  return *this;
}

Buffer &Buffer::flush(const VkDeviceSize offset, const VkDeviceSize size) {
  assert(m_handle != VK_NULL_HANDLE && m_mappedMemory);
  vmaFlushAllocation(m_memoryAllocator, m_allocation, offset, size);
  return *this;
}

//
// (private):
//

Buffer::Buffer(const VmaAllocator memoryAllocator, const VkDeviceSize size,
               const VkBufferUsageFlags bufferUsage,
               const VmaAllocationCreateFlags allocationFlags,
               const VmaMemoryUsage memoryUsage)
    : m_memoryAllocator{memoryAllocator} {
  const VkBufferCreateInfo bufferCreateInfo{
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = bufferUsage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
  };
  const VmaAllocationCreateInfo allocationCreateInfo{
    .flags = allocationFlags,
    .usage = memoryUsage,
  };

  VmaAllocationInfo allocationInfo{};
  VK_CHECK(vmaCreateBuffer(m_memoryAllocator, &bufferCreateInfo,
                           &allocationCreateInfo, &m_handle, &m_allocation,
                           &allocationInfo));
  m_size = allocationInfo.size;
}

void Buffer::_destroy() noexcept {
  if (m_handle != VK_NULL_HANDLE) {
    unmap();
    vmaDestroyBuffer(m_memoryAllocator, m_handle, m_allocation);

    m_memoryAllocator = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
    m_handle = VK_NULL_HANDLE;
    m_size = 0;
    m_mappedMemory = nullptr;
  }
}

} // namespace rhi
