#include "math/AABB.hpp"
#include "glm/ext/matrix_float4x4.hpp"

AABB AABB::create(const glm::vec3 &center, const glm::vec3 &halfExtents) {
  return {.min = center - halfExtents, .max = center + halfExtents};
}

glm::vec3 AABB::getCenter() const { return (max + min) * 0.5f; }
glm::vec3 AABB::getExtent() const { return max - min; }
float AABB::getRadius() const { return glm::length(getExtent() * 0.5f); }

AABB AABB::transform(const glm::mat4 &m) const {
  // https://dev.theomader.com/transform-bounding-boxes/

  const auto xa = m[0] * min.x;
  const auto xb = m[0] * max.x;

  const auto ya = m[1] * min.y;
  const auto yb = m[1] * max.y;

  const auto za = m[2] * min.z;
  const auto zb = m[2] * max.z;

  return {
    .min = {glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + m[3]},
    .max = {glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + m[3]},
  };
}

//
// Helper:
//

bool isUninitialized(const AABB &aabb) {
  constexpr auto kMaxFloat = std::numeric_limits<float>::max();
  constexpr auto kMinFloat = std::numeric_limits<float>::min();
  constexpr auto e = glm::vec3{std::numeric_limits<float>::epsilon()};

  return glm::all(glm::lessThan(glm::abs(aabb.min - kMaxFloat), e)) &&
         glm::all(glm::lessThan(glm::abs(aabb.max - kMinFloat), e));
}
