#include "renderer/Mesh.hpp"
#include "renderer/Material.hpp"
#include "spdlog/spdlog.h"

#include <algorithm> // all_of

namespace gfx {

//
// Mesh class:
//

const VertexFormat &Mesh::getVertexFormat() const { return *m_vertexFormat; }
const rhi::VertexBuffer *Mesh::getVertexBuffer() const {
  return m_vertexBuffer.get();
}
const rhi::IndexBuffer *Mesh::getIndexBuffer() const {
  return m_indexBuffer.get();
}

const std::vector<SubMesh> &Mesh::getSubMeshes() const { return m_subMeshes; }

bool Mesh::isSkeletal() const { return !m_inverseBindPose.empty(); }
const Joints &Mesh::getInverseBindPose() const { return m_inverseBindPose; }

const AABB &Mesh::getAABB() const { return m_aabb; }

//
// Builder class:
//

using Builder = Mesh::Builder;

Builder &Builder::setVertexFormat(std::shared_ptr<VertexFormat> vertexFormat) {
  m_vertexFormat = std::move(vertexFormat);
  return *this;
}
Builder &
Builder::setVertexBuffer(std::shared_ptr<rhi::VertexBuffer> vertexBuffer) {
  m_vertexBuffer = std::move(vertexBuffer);
  return *this;
}
Builder &
Builder::setIndexBuffer(std::shared_ptr<rhi::IndexBuffer> indexBuffer) {
  m_indexBuffer = std::move(indexBuffer);
  return *this;
}

Builder &Builder::setInverseBindPose(Joints joints) {
  m_inverseBindPose = std::move(joints);
  return *this;
}

Builder &Builder::setTopology(const rhi::PrimitiveTopology topology) {
  m_topology = topology;
  return *this;
}
Builder &Builder::beginSubMesh(const uint32_t vertexOffset,
                               const uint32_t numVertices,
                               std::shared_ptr<Material> material, AABB aabb) {
  if (!material) {
    SPDLOG_WARN("No material.");
  } else if (material && !isSurface(*material)) {
    SPDLOG_WARN("Invalid material domain.");
    material = {};
  }

  m_subMeshes.emplace_back(SubMesh{
    .topology = m_topology,
    .vertexOffset = vertexOffset,
    .numVertices = numVertices,
    .material = material,
    .aabb = std::move(aabb),
  });
  return *this;
}
Builder &Builder::addLOD(const uint32_t indexOffset,
                         const uint32_t numIndices) {
  m_subMeshes.back().lod.push_back({indexOffset, numIndices});
  return *this;
}

Builder &Builder::setAABB(AABB aabb) {
  m_aabb = std::move(aabb);
  return *this;
}

Mesh Builder::build() {
  Mesh m;
  m.m_vertexFormat = m_vertexFormat;
  m.m_vertexBuffer = m_vertexBuffer;
  m.m_indexBuffer = m_indexBuffer;

  [[maybe_unused]] static const auto withoutLOD = [](const gfx::SubMesh &sm) {
    return sm.lod.empty();
  };
  assert(m_indexBuffer || std::ranges::all_of(m_subMeshes, withoutLOD));
  if (isUninitialized(m_aabb)) m_aabb = m_subMeshes.front().aabb;

  m.m_subMeshes = std::move(m_subMeshes);
  m.m_inverseBindPose = std::move(m_inverseBindPose);
  m.m_aabb = m_aabb;

  return m;
}

//
// Helper:
//

rhi::GeometryInfo getGeometryInfo(const Mesh &mesh, const SubMesh &subMesh) {
  rhi::GeometryInfo geometryInfo{
    .vertexBuffer = mesh.getVertexBuffer(),
    .vertexOffset = subMesh.vertexOffset,
    .numVertices = subMesh.numVertices,
  };
  if (!subMesh.lod.empty()) {
    geometryInfo.indexBuffer = mesh.getIndexBuffer();
    const auto &lod = subMesh.lod.front();
    geometryInfo.indexOffset = lod.indexOffset;
    geometryInfo.numIndices = lod.numIndices;
  }
  return geometryInfo;
}

} // namespace gfx
