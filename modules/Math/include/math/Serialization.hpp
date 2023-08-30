#pragma once

#include "glm/fwd.hpp"

namespace glm {

template <class Archive, typename T>
void serialize(Archive &archive, vec<2, T> &v) {
  archive(v.x, v.y);
}
template <class Archive, typename T>
void serialize(Archive &archive, vec<3, T> &v) {
  archive(v.x, v.y, v.z);
}
template <class Archive, typename T>
void serialize(Archive &archive, vec<4, T> &v) {
  archive(v.x, v.y, v.z, v.w);
}

template <class Archive, typename T, qualifier Q>
void serialize(Archive &archive, qua<T, Q> &q) {
  archive(q.x, q.y, q.z, q.w);
}

template <class Archive, typename T, qualifier Q>
void serialize(Archive &archive, mat<2, 2, T, Q> &m) {
  archive(m[0], m[1]);
}
template <class Archive, typename T, qualifier Q>
void serialize(Archive &archive, mat<3, 3, T, Q> &m) {
  archive(m[0], m[1], m[2]);
}
template <class Archive, typename T, qualifier Q>
void serialize(Archive &archive, mat<4, 4, T, Q> &m) {
  archive(m[0], m[1], m[2], m[3]);
}

} // namespace glm
