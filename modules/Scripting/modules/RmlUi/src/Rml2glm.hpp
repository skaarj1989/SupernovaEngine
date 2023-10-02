#pragma once

#include "RmlUi/Core/Types.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

[[nodiscard]] inline glm::ivec2 to_glm(const Rml::Vector2i v) {
  return {v.x, v.y};
}
[[nodiscard]] inline glm::vec2 to_glm(const Rml::Vector2f v) {
  return {v.x, v.y};
}
[[nodiscard]] inline glm::vec3 to_glm(const Rml::Vector3f &v) {
  return {v.x, v.y, v.z};
}
[[nodiscard]] inline glm::vec4 to_glm(const Rml::Vector4f &v) {
  return {v.x, v.y, v.z, v.w};
}
