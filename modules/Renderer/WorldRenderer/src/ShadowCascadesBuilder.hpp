#pragma once

#include "renderer/Cascade.hpp"
#include <vector>

namespace gfx {

class PerspectiveCamera;

[[nodiscard]] std::vector<Cascade>
buildCascades(const PerspectiveCamera &, const glm::vec3 &lightDirection,
              uint32_t numCascades, const float lambda,
              const uint32_t shadowMapSize);

} // namespace gfx
