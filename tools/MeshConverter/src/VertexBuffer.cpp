#include "VertexBuffer.hpp"

void VertexBuffer::reserve(std::size_t count) {
  buffer.reserve(getStride() * count);
}

std::size_t VertexBuffer::getStride() const { return info.getStride(); }
std::size_t VertexBuffer::getNumVertices() const {
  return buffer.empty() ? 0 : buffer.size() / getStride();
}

ByteBuffer::value_type *VertexBuffer::at(std::size_t offset) {
  return buffer.data() + (getStride() * offset);
}

std::size_t VertexBuffer::insert(std::size_t count) {
  const auto offset = getNumVertices();
  buffer.insert(buffer.cend(), getStride() * count, 0);
  return offset;
}
