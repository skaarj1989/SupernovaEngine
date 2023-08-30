#pragma once

#include "AABB.hpp"
#include "Sphere.hpp"

[[nodiscard]] bool isPointInside(const glm::vec3 &, const AABB &);
[[nodiscard]] bool isPointInside(const glm::vec3 &, const Sphere &);

[[nodiscard]] bool intersects(const Sphere &, const Sphere &);
[[nodiscard]] bool intersects(const AABB &, const AABB &);
[[nodiscard]] bool intersects(const Sphere &, const AABB &);
