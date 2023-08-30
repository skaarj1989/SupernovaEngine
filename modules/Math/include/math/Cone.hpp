#pragma once

#include "glm/ext/vector_float3.hpp"

struct Cone {
  glm::vec3 T{0.0f}; // Cone tip.
  float h{0.0f};     // Height of the cone.
  glm::vec3 d{0.0f}; // Direction of the cone.
  float r{0.0f};     // Bottom radius of the cone.
};
