#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include <vector>
#include <span>

struct AABB;

class DebugDraw {
public:
  DebugDraw &addPoint(const glm::vec3 &position, const float size,
                      const glm::vec3 &color = glm::vec3{1.0f});
  DebugDraw &addPoint(const glm::vec3 &position, const float size,
                      const uint32_t color);

  DebugDraw &addLine(const glm::vec3 &origin, const glm::vec3 &end,
                     const glm::vec3 &color = glm::vec3{1.0f});
  DebugDraw &addLine(const glm::vec3 &origin, const glm::vec3 &end,
                     const uint32_t color);

  /** Circle on XY plane. */
  DebugDraw &addCircle(const float radius,
                       const glm::vec3 &color = glm::vec3{1.0f},
                       const glm::mat4 & = glm::mat4{1.0f});
  /** Sphere as 3 circles. */
  DebugDraw &addSphere(const float radius,
                       const glm::vec3 &color = glm::vec3{1.0f},
                       const glm::mat4 & = glm::mat4{1.0f});
  DebugDraw &addAABB(const AABB &, const glm::vec3 &color = glm::vec3{1.0f});
  DebugDraw &addFrustum(const glm::mat4 &inversedViewProj,
                        const glm::vec3 &color = glm::vec3{1.0f});

  bool empty() const;
  DebugDraw &clear();

  // TODO: More shapes.

  // ---

  struct Vertex {
    glm::vec4 position;
    uint32_t color;
  };
  using VertexList = std::vector<Vertex>;
  using IndexList = std::vector<uint32_t>;

  struct Range {
    uint32_t offset{0};
    uint32_t count{0};
  };
  class MeshRegistry {
  public:
    struct TriangleMesh {
      Range vertices;
      Range indices;
    };

    [[nodiscard]] uint32_t create(std::span<const Vertex>,
                                  std::span<const uint32_t> indices = {});
    const TriangleMesh &getTriangleMesh(const uint32_t index) const;

    const VertexList &getVertexList() const;
    const IndexList &getIndexList() const;

  private:
    VertexList m_vertices;
    IndexList m_indices;
    std::vector<TriangleMesh> m_meshes;
  };

  struct Instance {
    glm::mat4 modelMatrix{1.0f};
    glm::vec4 color{1.0f};
  };
  using InstanceList = std::vector<Instance>;

  struct MeshBatch {
    MeshRegistry *registry{nullptr};
    InstanceList instances;
    struct DrawInfo {
      const MeshRegistry::TriangleMesh *mesh{nullptr};
      Range instances;
    };
    std::vector<DrawInfo> drawInfo;

    void add(const MeshRegistry::TriangleMesh *, std::span<const Instance>);
  };

  struct Primitives {
    VertexList points;
    VertexList lines;
    MeshBatch meshes;

    [[nodiscard]] bool empty() const;
    [[nodiscard]] std::size_t size() const;
    void clear();
  };

  const Primitives &getPrimitives() const;

  DebugDraw &set(const MeshBatch &);

private:
  Primitives m_primitives;
};
