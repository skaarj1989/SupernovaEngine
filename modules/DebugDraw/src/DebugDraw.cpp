#include "DebugDraw.hpp"
#include "math/Frustum.hpp"
#include "math/Color.hpp"
#include "MergeVector.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp" // rotate
#include <iterator>

//
// DebugDraw class:
//

DebugDraw &DebugDraw::addPoint(const glm::vec3 &position, float size,
                               const glm::vec3 &color) {
  return addPoint(position, size, math::convertRGB(color));
}
DebugDraw &DebugDraw::addPoint(const glm::vec3 &position, float size,
                               uint32_t color) {
  m_primitives.points.emplace_back(glm::vec4{position, size}, color);
  return *this;
}
DebugDraw &DebugDraw::addLine(const glm::vec3 &origin, const glm::vec3 &end,
                              const glm::vec3 &color) {
  return addLine(origin, end, math::convertRGB(color));
}
DebugDraw &DebugDraw::addLine(const glm::vec3 &origin, const glm::vec3 &end,
                              uint32_t color) {
  m_primitives.lines.insert(m_primitives.lines.end(),
                            {{{origin, 1.0f}, color}, {{end, 1.0f}, color}});
  return *this;
}

DebugDraw &DebugDraw::addCircle(float radius, const glm::vec3 &color,
                                const glm::mat4 &model) {
  constexpr auto kSegments = 32;
  constexpr auto kIncrement = 2.0f * glm::pi<float>() / float{kSegments};
  static const auto kSinInc = glm::sin(kIncrement);
  static const auto kCosInc = glm::cos(kIncrement);

  glm::vec3 r1{1.0f, 0.0f, 0.0f};
  glm::vec3 v1{radius * r1};
  for (auto i = 0; i < kSegments; ++i) {
    glm::vec3 r2{0.0f};
    r2.x = kCosInc * r1.x - kSinInc * r1.y;
    r2.y = kSinInc * r1.x + kCosInc * r1.y;
    const auto v2 = radius * r2;
    addLine(model * glm::vec4{v1, 1.0f}, model * glm::vec4{v2, 1.0f}, color);

    r1 = r2;
    v1 = v2;
  }
  return *this;
}
DebugDraw &DebugDraw::addSphere(float radius, const glm::vec3 &color,
                                const glm::mat4 &m) {
  static const auto kZX =
    glm::rotate(glm::radians(90.0f), glm::vec3{1.0f, 0.0f, 0.0f});
  static const auto kYZ =
    glm::rotate(glm::radians(90.0f), glm::vec3{0.0f, 1.0f, 0.0f});

  return addCircle(radius, color, m) // XY
    .addCircle(radius, color, m * kZX)
    .addCircle(radius, color, m * kYZ);
}
DebugDraw &DebugDraw::addAABB(const AABB &aabb, const glm::vec3 &color) {
  // Coords taken from bullet btIDebugDraw::drawBox
  const auto &[min, max] = aabb;
  return
    // Front
    addLine(min, {max.x, min.y, min.z}, color)
      .addLine({max.x, min.y, min.z}, {max.x, max.y, min.z}, color)
      .addLine({max.x, max.y, min.z}, {min.x, max.y, min.z}, color)
      .addLine({min.x, max.y, min.z}, {min.x, min.y, min.z}, color)

      // Sides
      .addLine(min, {min.x, min.y, max.z}, color)
      .addLine({max.x, min.y, min.z}, {max.x, min.y, max.z}, color)
      .addLine({max.x, max.y, min.z}, max, color)
      .addLine({min.x, max.y, min.z}, {min.x, max.y, max.z}, color)

      // Back
      .addLine({min.x, min.y, max.z}, {max.x, min.y, max.z}, color)
      .addLine({max.x, min.y, max.z}, {max.x, max.y, max.z}, color)
      .addLine(max, {min.x, max.y, max.z}, color)
      .addLine({min.x, max.y, max.z}, {min.x, min.y, max.z}, color);
}
DebugDraw &DebugDraw::addFrustum(const glm::mat4 &inversedViewProj,
                                 const glm::vec3 &color) {
  const auto corners = Frustum::buildWorldSpaceCorners(inversedViewProj);
  return
    // Near:
    addLine(corners[4], corners[5], color)
      .addLine(corners[5], corners[6], color)
      .addLine(corners[6], corners[7], color)
      .addLine(corners[7], corners[4], color)

      // Sides:
      .addLine(corners[0], corners[4], color)
      .addLine(corners[1], corners[5], color)
      .addLine(corners[2], corners[6], color)
      .addLine(corners[3], corners[7], color)

      // Far:
      .addLine(corners[0], corners[1], color)
      .addLine(corners[1], corners[2], color)
      .addLine(corners[2], corners[3], color)
      .addLine(corners[3], corners[0], color);
}

bool DebugDraw::empty() const { return m_primitives.empty(); }
DebugDraw &DebugDraw::clear() {
  m_primitives.clear();
  return *this;
}

const DebugDraw::Primitives &DebugDraw::getPrimitives() const {
  return m_primitives;
}

DebugDraw &DebugDraw::set(const MeshBatch &meshes) {
  m_primitives.meshes = meshes;
  return *this;
}

//
// DebugDraw::MeshRegistry class:
//

uint32_t DebugDraw::MeshRegistry::create(std::span<const Vertex> vertices,
                                         std::span<const uint32_t> indices) {
  const auto id = m_meshes.size();

  m_meshes.push_back(TriangleMesh{
    .vertices =
      {
        .offset = uint32_t(m_vertices.size()),
        .count = uint32_t(vertices.size()),
      },
    .indices =
      {
        .offset = indices.empty() ? 0u : uint32_t(m_indices.size()),
        .count = uint32_t(indices.size()),
      },
  });
  insert(m_vertices, vertices);
  insert(m_indices, indices);

  return static_cast<uint32_t>(id);
}

const DebugDraw::MeshRegistry::TriangleMesh &
DebugDraw::MeshRegistry::getTriangleMesh(uint32_t index) const {
  return m_meshes[index];
}

const DebugDraw::VertexList &DebugDraw::MeshRegistry::getVertexList() const {
  return m_vertices;
}
const DebugDraw::IndexList &DebugDraw::MeshRegistry::getIndexList() const {
  return m_indices;
}

//
// DebugDraw::MeshBatch struct:
//

void DebugDraw::MeshBatch::add(const MeshRegistry::TriangleMesh *mesh,
                               std::span<const Instance> instances_) {
  drawInfo.emplace_back(mesh, DebugDraw::Range{
                                uint32_t(instances.size()),
                                uint32_t(instances_.size()),
                              });
  insert(instances, instances_);
}

//
// DebugDraw::Primitives struct:
//

bool DebugDraw::Primitives::empty() const {
  return points.empty() && lines.empty() && meshes.drawInfo.empty();
}
std::size_t DebugDraw::Primitives::size() const {
  return points.size() + lines.size() + meshes.drawInfo.size();
}
void DebugDraw::Primitives::clear() {
  points.clear();
  lines.clear();
  meshes = {};
}
