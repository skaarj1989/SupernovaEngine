#include "renderer/Light.hpp"
#include "math/Math.hpp"
#include "math/Frustum.hpp"
#include "Transform.hpp"

#include "tracy/Tracy.hpp"

namespace gfx {

std::vector<const Light *> getVisibleLights(std::span<const Light *> lights,
                                            const Frustum &frustum) {
  ZoneScoped;

  std::vector<const Light *> visibleLights;
  visibleLights.reserve(lights.size());
  for (const auto *light : lights)
    if (isLightInFrustum(*light, frustum)) visibleLights.emplace_back(light);
  return visibleLights;
}
bool isLightInFrustum(const Light &light, const Frustum &frustum) {
  switch (light.type) {
    using enum LightType;

  case Directional:
    return true;
  case Spot:
    return frustum.testCone(toCone(light));
  case Point:
    return frustum.testSphere(toSphere(light));
  }
  assert(false);
  return false;
}

RawCamera buildSpotLightMatrix(const glm::vec3 &position,
                               const glm::vec3 &direction, float fov,
                               float range) {
  ZoneScoped;

  auto projection = glm::perspective(glm::radians(fov), 1.0f, 1.0f, range);
  projection[1][1] *= -1.0f;

  const auto view =
    glm::lookAt(position, position + direction, calculateUpVector(direction));

  return {
    .view = view,
    .projection = projection,
    .viewProjection = projection * view,
  };
}
RawCamera buildSpotLightMatrix(const gfx::Light &spotLight) {
  assert(spotLight.type == gfx::LightType::Spot);
  return buildSpotLightMatrix(spotLight.position, spotLight.direction,
                              spotLight.outerConeAngle * 2.0f, spotLight.range);
}

RawCamera buildPointLightMatrix(rhi::CubeFace face, const glm::vec3 &position,
                                float far) {
  ZoneScoped;

  constexpr auto kAspectRatio = 1.0f;
  constexpr auto kFov = glm::half_pi<float>(); // == glm::radians(90.0f)
  const auto projection = glm::perspective(kFov, kAspectRatio, 0.1f, far);

  // clang-format off
  static constexpr glm::vec3 kTargetVectors[]{
    { 1.0f, 0.0f, 0.0f}, // +X
    {-1.0f, 0.0f, 0.0f}, // -X
    { 0.0f, 1.0f, 0.0f}, // +Y
    { 0.0f,-1.0f, 0.0f}, // -Y
    { 0.0f, 0.0f, 1.0f}, // +Z
    { 0.0f, 0.0f,-1.0f}  // -Z
  };
  static constexpr glm::vec3 kUpVectors[]{
    { 0.0f,-1.0f, 0.0f }, // +X
    { 0.0f,-1.0f, 0.0f }, // -X
    { 0.0f, 0.0f, 1.0f }, // +Y
    { 0.0f, 0.0f,-1.0f }, // -Y
    { 0.0f,-1.0f, 0.0f }, // +Z
    { 0.0f,-1.0f, 0.0f }  // -Z
  };
  //  clang-format on

  const auto faceIndex = std::to_underlying(face);
  const auto view =
    glm::lookAt(position, position + kTargetVectors[faceIndex], kUpVectors[faceIndex]);

  return RawCamera{
    .view = view,
    .projection = projection,
    .viewProjection = projection * view,
  };
}

Sphere toSphere(const Light &light) {
  assert(light.type == LightType::Point);
  return {.c = light.position, .r = light.range};
}
Cone toCone(const Light &light) {
  assert(light.type == LightType::Spot);

  const auto coneRadius =
    glm::tan(glm::radians(light.outerConeAngle)) * light.range;
  return {
    .T = light.position,
    .h = light.range,
    .d = light.direction,
    .r = coneRadius,
  };
}

float calculateLightRadius(const glm::vec3 &lightColor) {
  constexpr auto kConstant = 1.0f;
  constexpr auto kLinear = 0.7f;
  constexpr auto kQuadratic = 1.8f;

  return (-kLinear +
          std::sqrtf(kLinear * kLinear -
                     4.0f * kQuadratic *
                       (kConstant - (256.0f / 5.0f) * max3(lightColor)))) /
         (2.0f * kQuadratic);
}

} // namespace gfx
