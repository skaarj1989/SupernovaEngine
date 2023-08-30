#pragma once

#include "ScopedEnumFlags.hpp"
#include "Transform.hpp"
#include "ClippingPlanes.hpp"
#include "math/Frustum.hpp"

namespace gfx {

class PerspectiveCamera {
public:
  PerspectiveCamera() = default;
  PerspectiveCamera(const PerspectiveCamera &) = default;
  PerspectiveCamera(PerspectiveCamera &&) noexcept = default;
  ~PerspectiveCamera() = default;

  PerspectiveCamera &operator=(const PerspectiveCamera &) = default;
  PerspectiveCamera &operator=(PerspectiveCamera &&) noexcept = default;

  PerspectiveCamera &invertY(bool);

  PerspectiveCamera &freezeFrustum(bool);

  PerspectiveCamera &setPerspective(float fov, float aspectRatio,
                                    const ClippingPlanes &);

  PerspectiveCamera &setFov(float);
  PerspectiveCamera &setAspectRatio(float);
  PerspectiveCamera &setClippingPlanes(const ClippingPlanes &);

  PerspectiveCamera &setPosition(const glm::vec3 &);
  PerspectiveCamera &setOrientation(glm::quat);
  PerspectiveCamera &fromTransform(const Transform &);

  [[nodiscard]] bool isFrustumFreezed() const;

  [[nodiscard]] float getFov() const;
  [[nodiscard]] float getAspectRatio() const;
  [[nodiscard]] ClippingPlanes getClippingPlanes() const;

  [[nodiscard]] glm::vec3 getPosition() const;
  [[nodiscard]] glm::quat getOrientation() const;

  [[nodiscard]] float getPitch() const;
  [[nodiscard]] float getYaw() const;
  [[nodiscard]] float getRoll() const;

  [[nodiscard]] glm::vec3 getRight() const;
  [[nodiscard]] glm::vec3 getUp() const;
  [[nodiscard]] glm::vec3 getForward() const;

  [[nodiscard]] const glm::mat4 &getView() const;
  [[nodiscard]] const glm::mat4 &getProjection() const;
  [[nodiscard]] const glm::mat4 &getViewProjection() const;

  [[nodiscard]] const Frustum &getFrustum() const;

  // ---

  template <class Archive> void save(Archive &archive) const {
    archive(m_invertedY, m_fov, m_aspectRatio, m_clippingPlanes, m_position,
            m_orientation);
  }
  template <class Archive> void load(Archive &archive) {
    archive(m_invertedY, m_fov, m_aspectRatio, m_clippingPlanes, m_position,
            m_orientation);
    m_dirty = DirtyFlags::All;
  }

private:
  void _update() const;

private:
  float m_fov{60.0f};
  float m_aspectRatio{1.33f};
  ClippingPlanes m_clippingPlanes{0.1f, 100.0f};

  glm::vec3 m_position{0.0f, 0.0f, 0.0f};
  glm::quat m_orientation{1.0f, 0.0f, 0.0f, 0.0f};

  mutable glm::mat4 m_projection{1.0f};
  mutable glm::mat4 m_view{1.0f}; // World -> View.
  mutable glm::mat4 m_viewProjection{1.0f};

  mutable Frustum m_frustum;

  enum class DirtyFlags : uint8_t {
    None = 0,
    View = 1 << 0,
    Projection = 1 << 1,
    All = View | Projection,
    FrustumUnfreezed = 1 << 2,
  };
  mutable DirtyFlags m_dirty{DirtyFlags::All};

  bool m_invertedY{true};
  bool m_freezeFrustum{false};
};

template <> struct has_flags<PerspectiveCamera::DirtyFlags> : std::true_type {};

} // namespace gfx
