#pragma once

#include "rhi/GeometryInfo.hpp"
#include "math/AABB.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>
#include <vector>

namespace rhi {
class VertexBuffer;
class IndexBuffer;
} // namespace rhi

namespace gfx {

class Material;

struct SubMesh {
  rhi::PrimitiveTopology topology{rhi::PrimitiveTopology::TriangleList};
  uint32_t vertexOffset{0};
  uint32_t numVertices{0};
  struct LOD {
    uint32_t indexOffset{0};
    uint32_t numIndices{0};
  };
  std::vector<LOD> lod;
  const std::shared_ptr<Material> material;
  AABB aabb{}; // Local-space.
};

using Joints = std::vector<glm::mat4>;

class VertexFormat;

class Mesh {
public:
  Mesh(Mesh &&) noexcept = default;
  Mesh(const Mesh &) = delete;
  virtual ~Mesh() = default;

  Mesh &operator=(const Mesh &) = delete;
  Mesh &operator=(Mesh &&) noexcept = default;

  [[nodiscard]] const VertexFormat &getVertexFormat() const;
  [[nodiscard]] const rhi::VertexBuffer *getVertexBuffer() const;
  [[nodiscard]] const rhi::IndexBuffer *getIndexBuffer() const;

  [[nodiscard]] const std::vector<SubMesh> &getSubMeshes() const;

  [[nodiscard]] bool isSkeletal() const;
  [[nodiscard]] const Joints &getInverseBindPose() const;

  [[nodiscard]] const AABB &getAABB() const;

  class Builder {
  public:
    Builder() = default;
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    Builder &setVertexFormat(std::shared_ptr<VertexFormat>);
    Builder &setVertexBuffer(std::shared_ptr<rhi::VertexBuffer>);
    Builder &setIndexBuffer(std::shared_ptr<rhi::IndexBuffer>);

    Builder &setInverseBindPose(Joints);

    Builder &setTopology(const rhi::PrimitiveTopology);
    Builder &beginSubMesh(const uint32_t vertexOffset,
                          const uint32_t numVertices, std::shared_ptr<Material>,
                          AABB);
    Builder &addLOD(const uint32_t indexOffset, const uint32_t numIndices);

    Builder &setAABB(AABB);

    [[nodiscard]] Mesh build();

  private:
    std::shared_ptr<VertexFormat> m_vertexFormat;
    std::shared_ptr<rhi::VertexBuffer> m_vertexBuffer;
    std::shared_ptr<rhi::IndexBuffer> m_indexBuffer;

    rhi::PrimitiveTopology m_topology{rhi::PrimitiveTopology::TriangleList};
    std::vector<SubMesh> m_subMeshes;
    Joints m_inverseBindPose;

    AABB m_aabb{};
  };

private:
  Mesh() = default; // Called by the Builder class.

private:
  std::shared_ptr<VertexFormat> m_vertexFormat;
  std::shared_ptr<rhi::VertexBuffer> m_vertexBuffer;
  std::shared_ptr<rhi::IndexBuffer> m_indexBuffer;

  std::vector<SubMesh> m_subMeshes;
  Joints m_inverseBindPose;
  AABB m_aabb{}; // Local-space.
};

[[nodiscard]] rhi::GeometryInfo getGeometryInfo(const Mesh &, const SubMesh &);

} // namespace gfx
