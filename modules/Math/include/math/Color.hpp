#pragma once

#include "math/Math.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_uint4_sized.hpp"
#include "glm/ext/vector_uint3_sized.hpp"

namespace math {

[[nodiscard]] constexpr uint32_t convertRGBA(const glm::u8vec4 &c) {
  return c[3] << 24 | c[2] << 16 | c[1] << 8 | c[0];
}
[[nodiscard]] constexpr uint32_t convertRGBA(const glm::vec4 &c) {
  return convertRGBA(
    static_cast<glm::u8vec4>(c * float(std::numeric_limits<uint8_t>::max())));
}
[[nodiscard]] constexpr uint32_t convertRGB(const glm::u8vec3 &c) {
  return convertRGBA(glm::u8vec4{c, std::numeric_limits<uint8_t>::max()});
}
[[nodiscard]] constexpr uint32_t convertRGB(const glm::vec3 &c) {
  return convertRGBA(glm::vec4{c, 1.0f});
}

[[nodiscard]] constexpr glm::vec3 convertRGB(uint32_t color) {
  constexpr uint32_t kRedBits{0xFF'00'00'00};
  const auto r = normalize(uint8_t((color & kRedBits) >> 24));
  constexpr uint32_t kGreenBits{0x00'FF'00'00};
  const auto g = normalize(uint8_t((color & kGreenBits) >> 16));
  constexpr uint32_t kBlueBits{0x00'00'FF'00};
  const auto b = normalize(uint8_t((color & kBlueBits) >> 8));
  return glm::vec3{r, g, b};
}
[[nodiscard]] constexpr glm::vec4 convertRGBA(uint32_t color) {
  constexpr uint32_t kAlphaBits{0x00'00'00'FF};
  const auto a = normalize(uint8_t(color & kAlphaBits));
  return glm::vec4{convertRGB(color), a};
}

} // namespace math
