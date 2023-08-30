#pragma once

#include "VertexInfo.hpp"
#include "ByteBuffer.hpp"

struct VertexBuffer {
  VertexInfo info;
  ByteBuffer buffer;

  void reserve(std::size_t count);

  [[nodiscard]] std::size_t getStride() const;
  [[nodiscard]] std::size_t getNumVertices() const;

  // @param offset In vertices.
  // @return The pointer to the vertex.
  [[nodiscard]] ByteBuffer::value_type *at(std::size_t offset);

  // @return The offset to the first inserted vertex.
  [[nodiscard]] std::size_t insert(std::size_t count);
};
