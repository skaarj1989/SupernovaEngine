#include "ai2glm.hpp"
#include "glm/gtc/type_ptr.hpp" // make_mat4

glm::vec2 to_vec2(const aiVector3D &v) { return {v.x, v.y}; }
glm::vec3 to_vec3(const aiVector3D &v) { return {v.x, v.y, v.z}; }
glm::vec4 to_vec4(const aiColor4D &v) { return {v.r, v.g, v.b, v.a}; }
glm::mat4 to_mat4(const aiMatrix4x4 &m) {
  return glm::transpose(glm::make_mat4(&m.a1));
}
glm::quat to_quat(const aiQuaternion &q) { return {q.w, q.x, q.y, q.z}; }

AABB to_aabb(const aiAABB &aabb) {
  return {
    .min = to_vec3(aabb.mMin),
    .max = to_vec3(aabb.mMax),
  };
}
