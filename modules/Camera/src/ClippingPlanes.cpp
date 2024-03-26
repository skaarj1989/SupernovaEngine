#include "ClippingPlanes.hpp"
#include "glm/ext/matrix_float4x4.hpp"

namespace gfx {

ClippingPlanes decomposeProjection(const glm::mat4 &projection) {
  const auto m22 = projection[2][2];
  const auto m32 = projection[3][2];

#if GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_ZO_BIT
  const auto n = m32 / m22;
  const auto f = m32 / (m22 + 1.0f);
#else
  const auto n = (2.0f * m32) / (2.0f * m22 - 2.0f);
  const auto f = ((m22 - 1.0f) * n) / (m22 + 1.0f);
#endif
  return {n, f};
}

} // namespace gfx
