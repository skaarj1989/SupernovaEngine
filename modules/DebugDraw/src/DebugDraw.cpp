#include "DebugDraw.hpp"
#include "math/Frustum.hpp"
#include "math/Color.hpp"
#include "glm/gtx/transform.hpp" // rotate

const DebugDraw::Primitives &DebugDraw::getPrimitives() const {
  return m_primitives;
}

void DebugDraw::addPoint(const glm::vec3 &position, float size,
                         const glm::vec3 &color) {
  addPoint(position, size, math::convertRGB(color));
}
void DebugDraw::addPoint(const glm::vec3 &position, float size,
                         uint32_t color) {
  m_primitives.points.emplace_back(glm::vec4{position, size}, color);
}
void DebugDraw::addLine(const glm::vec3 &origin, const glm::vec3 &end,
                        const glm::vec3 &color) {
  addLine(origin, end, math::convertRGB(color));
}
void DebugDraw::addLine(const glm::vec3 &origin, const glm::vec3 &end,
                        uint32_t color) {
  m_primitives.lines.insert(m_primitives.lines.end(),
                            {{{origin, 1.0f}, color}, {{end, 1.0f}, color}});
}

void DebugDraw::addCircle(float radius, const glm::vec3 &color,
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
}
void DebugDraw::addSphere(float radius, const glm::vec3 &color,
                          const glm::mat4 &m) {
  static const auto kZX =
    glm::rotate(glm::radians(90.0f), glm::vec3{1.0f, 0.0f, 0.0f});
  static const auto kYZ =
    glm::rotate(glm::radians(90.0f), glm::vec3{0.0f, 1.0f, 0.0f});

  addCircle(radius, color, m); // XY
  addCircle(radius, color, m * kZX);
  addCircle(radius, color, m * kYZ);
}
void DebugDraw::addAABB(const AABB &aabb, const glm::vec3 &color) {
  // Coords taken from bullet btIDebugDraw::drawBox
  const auto &[min, max] = aabb;

  // Front
  addLine(min, {max.x, min.y, min.z}, color);
  addLine({max.x, min.y, min.z}, {max.x, max.y, min.z}, color);
  addLine({max.x, max.y, min.z}, {min.x, max.y, min.z}, color);
  addLine({min.x, max.y, min.z}, {min.x, min.y, min.z}, color);

  // Sides
  addLine(min, {min.x, min.y, max.z}, color);
  addLine({max.x, min.y, min.z}, {max.x, min.y, max.z}, color);
  addLine({max.x, max.y, min.z}, max, color);
  addLine({min.x, max.y, min.z}, {min.x, max.y, max.z}, color);

  // Back
  addLine({min.x, min.y, max.z}, {max.x, min.y, max.z}, color);
  addLine({max.x, min.y, max.z}, {max.x, max.y, max.z}, color);
  addLine(max, {min.x, max.y, max.z}, color);
  addLine({min.x, max.y, max.z}, {min.x, min.y, max.z}, color);
}
void DebugDraw::addFrustum(const glm::mat4 &inversedViewProj,
                           const glm::vec3 &color) {
  const auto corners = Frustum::buildWorldSpaceCorners(inversedViewProj);

  // Near:

  addLine(corners[4], corners[5], color);
  addLine(corners[5], corners[6], color);
  addLine(corners[6], corners[7], color);
  addLine(corners[7], corners[4], color);

  // Sides:

  addLine(corners[0], corners[4], color);
  addLine(corners[1], corners[5], color);
  addLine(corners[2], corners[6], color);
  addLine(corners[3], corners[7], color);

  // Far:

  addLine(corners[0], corners[1], color);
  addLine(corners[1], corners[2], color);
  addLine(corners[2], corners[3], color);
  addLine(corners[3], corners[0], color);
}

bool DebugDraw::empty() const { return m_primitives.empty(); }
void DebugDraw::clear() { m_primitives.clear(); }
