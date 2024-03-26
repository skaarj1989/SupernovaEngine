#include "rhi/IndexBuffer.hpp"
#include <utility> // move
#include <cassert>

namespace rhi {

IndexType IndexBuffer::getIndexType() const { return m_indexType; }
Buffer::Stride IndexBuffer::getStride() const {
  return std::to_underlying(m_indexType);
}
VkDeviceSize IndexBuffer::getCapacity() const {
  assert(m_indexType != IndexType::Undefined);
  const auto stride = int32_t(m_indexType);
  return stride > 0 ? getSize() / stride : 0;
}

IndexBuffer::IndexBuffer(Buffer &&buffer, const IndexType indexType)
    : Buffer{std::move(buffer)}, m_indexType{indexType} {}

} // namespace rhi
