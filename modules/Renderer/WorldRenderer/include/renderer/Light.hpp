#pragma once

#include "math/Sphere.hpp"
#include "math/Cone.hpp"
#include "rhi/CubeFace.hpp"
#include "RawCamera.hpp"
#include <vector>
#include <span>

class Frustum;

namespace gfx {

enum class LightType : int32_t { Directional, Spot, Point };

struct Light {
  LightType type;
  bool visible{true};
  bool castsShadow{true};
  bool debugVolume{false};

  glm::vec3 position{0.0f};  // In world-space.
  glm::vec3 direction{0.0f}; // In world-space.

  glm::vec3 color{1.0f};
  float intensity{1.0f};
  float range{1.0f};

  float innerConeAngle{15.0f}; // In degrees.
  float outerConeAngle{35.0f}; // In degrees.
  // - Higher bias value causes shadow detachment
  // (decreases range when the shadow disappears).
  // - Lower value increases shadow acne.
  float shadowBias{0.005f};

  template <class Archive> void serialize(Archive &archive) {
    archive(type, visible, castsShadow, position, direction, color, intensity,
            range, innerConeAngle, outerConeAngle, shadowBias);
  }
};

static_assert(std::is_copy_constructible_v<Light>);

[[nodiscard]] std::vector<const Light *>
getVisibleLights(std::span<const Light *>, const Frustum &);
[[nodiscard]] bool isLightInFrustum(const Light &, const Frustum &);

[[nodiscard]] RawCamera buildSpotLightMatrix(const glm::vec3 &position,
                                             const glm::vec3 &direction,
                                             float fov, float range);
[[nodiscard]] RawCamera buildSpotLightMatrix(const Light &);

[[nodiscard]] RawCamera
buildPointLightMatrix(rhi::CubeFace, const glm::vec3 &position, float radius);

[[nodiscard]] Sphere toSphere(const Light &);
[[nodiscard]] Cone toCone(const Light &);

[[nodiscard]] float calculateLightRadius(const glm::vec3 &lightColor);

} // namespace gfx
