#include "rhi/VertexBuffer.hpp"
#include <utility> // move

namespace rhi {

Buffer::Stride VertexBuffer::getStride() const { return m_stride; }
VkDeviceSize VertexBuffer::getCapacity() const {
  return m_stride > 0 ? getSize() / m_stride : 0;
}

VertexBuffer::VertexBuffer(Buffer &&buffer, const Stride stride)
    : Buffer{std::move(buffer)}, m_stride{stride} {}

} // namespace rhi
