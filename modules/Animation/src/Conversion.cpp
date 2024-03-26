#include "animation/Conversion.hpp"

glm::mat4 to_mat4(const ozz::math::Float4x4 &m) {
  glm::mat4 result{};
  for (auto i = 0; i < decltype(result)::length(); ++i) {
    ozz::math::StorePtr(m.cols[i], &result[i].x);
  }
  return result;
}
