#include "BiasMatrix.hpp"
#include "glm/mat4x4.hpp"

namespace gfx {

// clang-format off
#if GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_ZO_BIT
// Vulkan, Depth [0, 1]
const glm::mat4 kBiasMatrix{
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.5, 0.5, 0.0, 1.0
};
#else
// OpenGL, Depth [-1, 1] (by default)
const glm::mat4 kBiasMatrix{
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 0.5, 0.0,
  0.5, 0.5, 0.5, 1.0
};
#endif
// clang-format on

} // namespace gfx
