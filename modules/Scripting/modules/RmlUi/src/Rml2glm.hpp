#pragma once

#include "RmlUi/Core/Types.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/ext/vector_float2.hpp"

[[nodiscard]] inline glm::ivec2 to_glm(const Rml::Vector2i v) {
  return {v.x, v.y};
}
[[nodiscard]] inline glm::vec2 to_glm(const Rml::Vector2f v) {
  return {v.x, v.y};
}
