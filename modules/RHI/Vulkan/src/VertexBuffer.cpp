#include "rhi/VertexBuffer.hpp"
#include <type_traits> // move

namespace rhi {

uint32_t VertexBuffer::getStride() const { return m_stride; }
VkDeviceSize VertexBuffer::getCapacity() const {
  return m_stride > 0 ? getSize() / m_stride : 0;
}

VertexBuffer::VertexBuffer(Buffer &&buffer, uint32_t stride)
    : Buffer{std::move(buffer)}, m_stride{stride} {}

} // namespace rhi
