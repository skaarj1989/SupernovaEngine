#include "Mesh.hpp"
#include <cassert>

namespace offline {

namespace {

[[nodiscard]] int32_t findIndexStride(std::size_t numIndices) {
  // Vulkan doesn't like indices smaller than 2 bytes.
  if (numIndices <= UINT16_MAX)
    return sizeof(uint16_t);
  else if (numIndices <= UINT32_MAX)
    return sizeof(uint32_t);

  assert(false);
  return 0;
}

} // namespace

//
// Mesh struct:
//

void Mesh::reserve(const SizeInfo &info) {
  vertices.reserve(info.numVertices);
  indices.reserve(info.numIndices);
  indexStride = findIndexStride(info.numIndices);
}
void Mesh::updateIndexStride() {
  indexStride = findIndexStride(indices.size());
}

SubMesh &Mesh::add(std::string_view name, const SizeInfo &info) {
  SubMesh subMesh{
    .name = name.data(),
    .vertexOffset = vertices.insert(info.numVertices),
    .numVertices = info.numVertices,
    .LODs =
      {
        {.indexOffset = indices.size(), .numIndices = info.numIndices},
      },
  };
  return subMeshes.emplace_back(std::move(subMesh));
}

ByteBuffer::value_type *Mesh::vertexAt(const SubMesh &subMesh,
                                       std::size_t offset) {
  return vertices.at(subMesh.vertexOffset + offset);
}

} // namespace offline
