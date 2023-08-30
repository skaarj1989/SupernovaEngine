#pragma once

#include "math/AABB.hpp"
#include <vector>

class DebugDraw {
public:
  struct Vertex {
    glm::vec4 position;
    uint32_t color;
  };
  struct Primitives {
    using Vertices = std::vector<Vertex>;

    Vertices points;
    Vertices lines;

    [[nodiscard]] bool empty() const { return points.empty() && lines.empty(); }
    void clear() {
      points.clear();
      lines.clear();
    }
    [[nodiscard]] auto size() const { return points.size() + lines.size(); }
  };

  const Primitives &getPrimitives() const;

  void addPoint(const glm::vec3 &position, float size,
                const glm::vec3 &color = glm::vec3{1.0f});
  void addPoint(const glm::vec3 &position, float size, uint32_t color);

  void addLine(const glm::vec3 &origin, const glm::vec3 &end,
               const glm::vec3 &color = glm::vec3{1.0f});
  void addLine(const glm::vec3 &origin, const glm::vec3 &end, uint32_t color);

  /** Circle on XY plane. */
  void addCircle(float radius, const glm::vec3 &color = glm::vec3{1.0f},
                 const glm::mat4 & = glm::mat4{1.0f});
  /** Sphere as 3 circles. */
  void addSphere(float radius, const glm::vec3 &color = glm::vec3{1.0f},
                 const glm::mat4 & = glm::mat4{1.0f});
  void addAABB(const AABB &, const glm::vec3 &color = glm::vec3{1.0f});
  void addFrustum(const glm::mat4 &inversedViewProj, const glm::vec3 &color);

  // TODO: More shapes.

  bool empty() const;
  void clear();

private:
  Primitives m_primitives;
};
