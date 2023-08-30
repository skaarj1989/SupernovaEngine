#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/common.hpp" // max, abs

[[nodiscard]] constexpr bool isPowerOf2(auto v) { return v && !(v & (v - 1)); }
[[nodiscard]] constexpr auto nextPowerOf2(auto v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

[[nodiscard]] constexpr float max3(const glm::vec3 &v) {
  return glm::max(glm::max(v.x, v.y), v.z);
}
template <typename T> [[nodiscard]] constexpr double normalize(T value) {
  return value < 0 ? -static_cast<double>(value) / std::numeric_limits<T>::min()
                   : static_cast<double>(value) / std::numeric_limits<T>::max();
}

template <typename T> [[nodiscard]] bool isApproximatelyEqual(T a, T b) {
  return glm::abs(a - b) < std::numeric_limits<T>::epsilon();
}
