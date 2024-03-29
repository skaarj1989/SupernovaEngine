#include "physics/Conversion.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/Math/Vec3.h"
#include "Jolt/Math/Vec4.h"
#include "Jolt/Math/Quat.h"
#include "Jolt/Math/Mat44.h"
#include "Jolt/Geometry/AABox.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include "math/AABB.hpp"
#include "glm/gtc/quaternion.hpp"

JPH::Vec3 to_Jolt(const glm::vec3 &v) { return {v.x, v.y, v.z}; }
JPH::Quat to_Jolt(const glm::quat &q) { return {q.x, q.y, q.z, q.w}; }

glm::vec3 to_glm(const JPH::Float3 &v) { return {v.x, v.y, v.z}; }
glm::vec3 to_glm(const JPH::Vec3 &v) { return {v.GetX(), v.GetY(), v.GetZ()}; }
glm::vec4 to_glm(const JPH::Vec4 &v) {
  return {v.GetX(), v.GetY(), v.GetZ(), v.GetW()};
}
glm::quat to_glm(const JPH::Quat &q) {
  return {q.GetW(), q.GetX(), q.GetY(), q.GetZ()};
}
glm::mat4 to_glm(const JPH::Mat44 &m) {
  return {
    to_glm(m.GetColumn4(0)),
    to_glm(m.GetColumn4(1)),
    to_glm(m.GetColumn4(2)),
    to_glm(m.GetColumn4(3)),
  };
}

AABB from_Jolt(const JPH::AABox &box) {
  return AABB{.min = to_glm(box.mMin), .max = to_glm(box.mMax)};
}
