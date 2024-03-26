#pragma once

#include "glm/fwd.hpp"
#include "glm/ext/vector_float3.hpp"
#include <compare>

struct AABB {
  glm::vec3 min{std::numeric_limits<float>::max()};
  glm::vec3 max{std::numeric_limits<float>::min()};

  static AABB create(const glm::vec3 &center, const glm::vec3 &halfExtents);

  [[nodiscard]] glm::vec3 getCenter() const;
  [[nodiscard]] glm::vec3 getExtent() const;
  [[nodiscard]] float getRadius() const;

  [[nodiscard]] AABB transform(const glm::mat4 &) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(min, max);
  }

  auto operator<=>(const AABB &) const = default;
};

[[nodiscard]] bool isUninitialized(const AABB &);
