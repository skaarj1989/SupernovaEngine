#include "ShadowCascadesBuilder.hpp"
#include "tracy/Tracy.hpp"

// https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/

// WARNING:
// Shadowmap shimmering may occur with certain clipping planes values of the
// PerspectiveCamera.

namespace gfx {

namespace {

using Splits = std::vector<float>;

[[nodiscard]] auto buildCascadeSplits(uint32_t numCascades, float lambda,
                                      float near, float clipRange) {
  assert(numCascades > 0);

  const auto minZ = near;
  const auto maxZ = near + clipRange;

  const auto range = maxZ - minZ;
  const auto ratio = maxZ / minZ;

  Splits cascadeSplits(numCascades);
  // Calculate split depths based on view camera frustum.
  // Based on method presented in:
  // https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
  for (auto i = 0; i < cascadeSplits.size(); ++i) {
    const auto p = float(i + 1) / float(cascadeSplits.size());
    const auto log = minZ * std::pow(ratio, p);
    const auto uniform = minZ + range * p;
    // glm::mix != std::lerp
    const auto d = std::lerp(uniform, log, lambda);
    cascadeSplits[i] = (d - near) / clipRange;
  }
  return cascadeSplits;
}

// Returns frustum corners in world space.
[[nodiscard]] auto buildFrustumCorners(const glm::mat4 &inversedViewProj,
                                       float splitDist, float lastSplitDist) {
  auto frustumCorners = Frustum::buildWorldSpaceCorners(inversedViewProj);

  for (auto i = 0; i < 4; ++i) {
    const auto cornerRay = frustumCorners[i + 4] - frustumCorners[i];
    const auto nearCornerRay = cornerRay * lastSplitDist;
    const auto farCornerRay = cornerRay * splitDist;
    frustumCorners[i + 4] = frustumCorners[i] + farCornerRay;
    frustumCorners[i] = frustumCorners[i] + nearCornerRay;
  }
  return frustumCorners;
}
[[nodiscard]] auto measureFrustum(const Frustum::Corners &frustumCorners) {
  ZoneScopedN("MeasureFrustum");

  glm::vec3 center{0.0f};
  for (const auto &p : frustumCorners) {
    center += p;
  }
  center /= float(frustumCorners.size());

  auto radius = 0.0f;
  for (const auto &p : frustumCorners) {
    const auto distance = glm::length(p - center);
    radius = glm::max(radius, distance);
  }
  radius = glm::ceil(radius * 16.0f) / 16.0f;

  return std::tuple{center, radius};
}

void eliminateShimmering(const glm::mat4 &view, glm::mat4 &projection,
                         uint32_t shadowMapSize) {
  ZoneScopedN("EliminateShimmering");

  const auto shadowMatrix = projection * view;
  auto shadowOrigin =
    glm::vec2{shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};
  shadowOrigin *= float(shadowMapSize) * 0.5f;

  const auto roundedOrigin = glm::round(shadowOrigin);
  auto roundOffset = roundedOrigin - shadowOrigin;
  roundOffset = roundOffset * 2.0f / float(shadowMapSize);
  projection[3] += glm::vec4{roundOffset, 0.0f, 0.0f};
}

[[nodiscard]] auto buildDirLightMatrix(const glm::mat4 &inversedViewProj,
                                       const glm::vec3 &lightDirection,
                                       uint32_t shadowMapSize, float splitDist,
                                       float lastSplitDist) {
  ZoneScopedN("BuildDirLightMatrix");

  const auto frustumCorners =
    buildFrustumCorners(inversedViewProj, splitDist, lastSplitDist);

  auto [center, radius] = measureFrustum(frustumCorners);
  AABB aabb{.min = glm::vec3{-radius}, .max = glm::vec3{radius}};

  const auto eye = center - glm::normalize(lightDirection) * -aabb.min.z;
  const auto view = glm::lookAt(eye, center, Transform::kUp);
  auto projection = glm::ortho(aabb.min.x, aabb.max.x, aabb.min.y, aabb.max.y,
                               0.0f, aabb.max.z - aabb.min.z);
  projection[1][1] *= -1.0f;
  eliminateShimmering(view, projection, shadowMapSize);

  return std::tuple{
    aabb,
    RawCamera{
      .view = view,
      .projection = projection,
    },
  };
}

} // namespace

std::vector<Cascade> buildCascades(const PerspectiveCamera &camera,
                                   const glm::vec3 &lightDirection,
                                   uint32_t numCascades, float lambda,
                                   uint32_t shadowMapSize) {
  ZoneScopedN("BuildCascades");

  numCascades = glm::clamp(numCascades, 1u, kMaxNumCascades);

  const auto [zNear, zFar] = camera.getClippingPlanes();
  const auto clipRange = zFar - zNear;
  const auto cascadeSplits =
    buildCascadeSplits(numCascades, lambda, zNear, clipRange);
  const auto inversedViewProj = glm::inverse(camera.getViewProjection());

  std::vector<Cascade> cascades(numCascades);
  auto lastSplitDist = 0.0f;
  for (auto i = 0; i < cascades.size(); ++i) {
    const auto splitDist = cascadeSplits[i];

    auto &[splitDepth, aabb, lightView] = cascades[i];
    splitDepth = (zNear + splitDist * clipRange) * -1.0f;
    std::tie(aabb, lightView) =
      buildDirLightMatrix(inversedViewProj, lightDirection, shadowMapSize,
                          splitDist, lastSplitDist);

    lastSplitDist = splitDist;
  }
  return cascades;
}

} // namespace gfx
