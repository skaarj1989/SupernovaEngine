#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <memory>

namespace gfx {

class VertexFormat;

struct Vertex1p1n1st {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;

  static std::shared_ptr<VertexFormat> getVertexFormat();
};

} // namespace gfx
