#include "math/CollisionDetection.hpp"
#include "math/AABB.hpp"
#include "math/Sphere.hpp"
#include "glm/vector_relational.hpp" // all
#include "glm/geometric.hpp"         // distance

// https://courses.cs.duke.edu//cps124/spring04/notes/12_collisions/collision_detection.pdf
// https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection

bool isPointInside(const glm::vec3 &point, const AABB &aabb) {
  return glm::all(glm::greaterThanEqual(point, aabb.min)) &&
         glm::all(glm::lessThanEqual(point, aabb.max));
}
bool isPointInside(const glm::vec3 &point, const Sphere &sphere) {
  return glm::distance(point, sphere.c) < sphere.r;
}

bool intersects(const Sphere &a, const Sphere &b) {
  return glm::distance(a.c, b.c) < (a.r + b.r);
}
bool intersects(const AABB &a, const AABB &b) {
  return glm::all(glm::lessThanEqual(a.min, b.max)) &&
         glm::all(glm::greaterThanEqual(a.max, b.min));
}
bool intersects(const Sphere &sphere, const AABB &aabb) {
  const auto closestPoint = glm::max(aabb.min, glm::min(sphere.c, aabb.max));
  return glm::distance(closestPoint, sphere.c) < sphere.r;
}
