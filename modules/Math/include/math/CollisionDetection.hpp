#pragma once

#include "glm/fwd.hpp"

struct AABB;
struct Sphere;

[[nodiscard]] bool isPointInside(const glm::vec3 &, const AABB &);
[[nodiscard]] bool isPointInside(const glm::vec3 &, const Sphere &);

[[nodiscard]] bool intersects(const Sphere &, const Sphere &);
[[nodiscard]] bool intersects(const AABB &, const AABB &);
[[nodiscard]] bool intersects(const Sphere &, const AABB &);
