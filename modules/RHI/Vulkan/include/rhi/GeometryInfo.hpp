#pragma once

#include "rhi/PrimitiveTopology.hpp"

namespace rhi {

class VertexBuffer;
class IndexBuffer;

// It's possible to draw without a vertexBuffer and indexBuffer
// (hence pointers instead of references).
struct GeometryInfo {
  PrimitiveTopology topology{PrimitiveTopology::TriangleList};
  const VertexBuffer *vertexBuffer{nullptr};
  uint32_t vertexOffset{0};
  uint32_t numVertices{0};
  const IndexBuffer *indexBuffer{nullptr};
  uint32_t indexOffset{0};
  uint32_t numIndices{0};
};

} // namespace rhi
