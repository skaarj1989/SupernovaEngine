#pragma once

#include "glm/ext/vector_float3.hpp"

struct CollisionStartedEvent {
  uint32_t other; // entityId
  glm::vec3 offset;
  glm::vec3 normal;
};
struct CollisionEndedEvent {
  uint32_t other; // entityId
};
