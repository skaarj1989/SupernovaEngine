#pragma once

#include "renderer/Cascade.hpp"
#include "PerspectiveCamera.hpp"
#include <vector>

namespace gfx {

[[nodiscard]] std::vector<Cascade>
buildCascades(const PerspectiveCamera &, const glm::vec3 &lightDirection,
              uint32_t numCascades, float lambda, uint32_t shadowMapSize);

} // namespace gfx
