#pragma once

#include "fg/Fwd.hpp"
#include "renderer/Cascade.hpp"
#include <vector>

namespace gfx {

struct ShadowBlock {
  std::vector<Cascade> cascades;
  std::vector<glm::mat4> spotLightViewProjections;
};
[[nodiscard]] FrameGraphResource uploadShadowBlock(FrameGraph &,
                                                   const ShadowBlock &);

} // namespace gfx
