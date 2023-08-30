#include "ai2ozz.hpp"

ozz::math::Float3 to_ozz(const aiVector3D &v) { return {v.x, v.y, v.z}; }
ozz::math::Transform to_ozz(const aiMatrix4x4 &m) {
  aiVector3D s;
  aiQuaternion r;
  aiVector3D t;
  m.Decompose(s, r, t);

  return ozz::math::Transform{
    .translation = to_ozz(t),
    .rotation = to_ozz(r),
    .scale = to_ozz(s),
  };
}
ozz::math::Quaternion to_ozz(const aiQuaternion &q) {
  return {q.x, q.y, q.z, q.w};
}
