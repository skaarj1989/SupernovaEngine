#pragma once

#include "glm/ext/vector_float4.hpp"

namespace gfx {

// Used in a lighting pass (as push_constant).
struct LightingSettings {
  glm::vec4 ambientLight;
  float IBLIntensity{1.0f};
  float GIIntensity{1.0f};
};
static_assert(sizeof(LightingSettings) < 128);

} // namespace gfx
