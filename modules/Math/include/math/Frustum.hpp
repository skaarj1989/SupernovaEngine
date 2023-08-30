#pragma once

#include "Plane.hpp"
#include "AABB.hpp"
#include "Sphere.hpp"
#include "Cone.hpp"
#include <array>

class Frustum {
public:
  using Planes = std::array<Plane, 6>;
  using Corners = std::array<glm::vec3, 8>;

  Frustum() = default;
  explicit Frustum(const glm::mat4 &);
  Frustum(const Frustum &) = default;
  Frustum(Frustum &&) noexcept = default;
  ~Frustum() = default;

  Frustum &operator=(const Frustum &) = default;
  Frustum &operator=(Frustum &&) noexcept = default;

  /**
   * @param [in] m
   *  if projection    => Planes in View-space.
   *  if viewProj      => Planes in World-space.
   *  if modelViewProj => Planes in Model-space.
   */
  void update(const glm::mat4 &m);

  [[nodiscard]] bool testPoint(const glm::vec3 &) const;
  [[nodiscard]] bool testAABB(const AABB &) const;
  [[nodiscard]] bool testSphere(const Sphere &) const;
  [[nodiscard]] bool testCone(const Cone &) const;

  [[nodiscard]] static Corners
  buildWorldSpaceCorners(const glm::mat4 &inversedViewProj);

private:
  Planes m_planes;
};
