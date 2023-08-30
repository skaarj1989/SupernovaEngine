#pragma once

#include "VertexBuffer.hpp"
#include "SubMesh.hpp"
#include "aiStringHash.hpp"

using aiInverseBindPoseMap =
  std::unordered_map<aiString, aiMatrix4x4, aiStringHash>;

namespace offline {

struct Mesh {
  struct SizeInfo {
    std::size_t numVertices{0};
    std::size_t numIndices{0};
  };
  void reserve(const SizeInfo &);
  void updateIndexStride();

  SubMesh &add(std::string_view name, const SizeInfo &);

  ByteBuffer::value_type *vertexAt(const SubMesh &, std::size_t offset = 0);

  VertexBuffer vertices;

  uint32_t indexStride{0};
  IndicesList indices;

  // In bytes
  struct BufferOffsets {
    std::size_t vertices{0};
    std::size_t indices{0};
    std::size_t inverseBindPose{0};
  };
  BufferOffsets byteOffsets;

  AABB aabb;
  std::vector<SubMesh> subMeshes;

  aiInverseBindPoseMap inverseBindPoseMap;
};

} // namespace offline
