#pragma once

#include "RmlUi/Core/Types.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

[[nodiscard]] inline Rml::Vector2i to_Rml(const glm::ivec2 v) {
  return {v.x, v.y};
}
[[nodiscard]] inline Rml::Vector2f to_Rml(const glm::vec2 v) {
  return {v.x, v.y};
}
[[nodiscard]] inline Rml::Vector3f to_Rml(const glm::vec3 v) {
  return {v.x, v.y, v.z};
}
[[nodiscard]] inline Rml::Vector4f to_Rml(const glm::vec4 v) {
  return {v.x, v.y, v.z, v.w};
}
