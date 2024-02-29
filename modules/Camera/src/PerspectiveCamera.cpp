#include "PerspectiveCamera.hpp"
#include "math/Math.hpp"
#include "glm/gtc/quaternion.hpp"

#include "tracy/Tracy.hpp"

namespace gfx {

PerspectiveCamera &PerspectiveCamera::freezeFrustum(bool freezed) {
  m_freezeFrustum = freezed;
  if (freezed == false) m_dirty |= DirtyFlags::FrustumUnfreezed;
  return *this;
}

PerspectiveCamera &PerspectiveCamera::setPerspective(float fov,
                                                     float aspectRatio,
                                                     const ClippingPlanes &cp) {
  return setFov(fov).setAspectRatio(aspectRatio).setClippingPlanes(cp);
}

PerspectiveCamera &PerspectiveCamera::setFov(float s) {
  if (!isApproximatelyEqual(m_fov, s)) {
    m_fov = s;
    m_dirty |= DirtyFlags::Projection;
  }
  return *this;
}
PerspectiveCamera &PerspectiveCamera::setAspectRatio(float s) {
  if (!isApproximatelyEqual(m_aspectRatio, s)) {
    m_aspectRatio = s;
    m_dirty |= DirtyFlags::Projection;
  }
  return *this;
}
PerspectiveCamera &
PerspectiveCamera::setClippingPlanes(const ClippingPlanes &cp) {
  if (cp != m_clippingPlanes) {
    m_clippingPlanes = cp;
    m_dirty |= DirtyFlags::Projection;
  }
  return *this;
}

PerspectiveCamera &PerspectiveCamera::setPosition(const glm::vec3 &v) {
  if (v != m_position) {
    m_position = v;
    m_dirty |= DirtyFlags::View;
  }
  return *this;
}
PerspectiveCamera &PerspectiveCamera::setOrientation(glm::quat q) {
  q = glm::normalize(q);
  if (q != m_orientation) {
    m_orientation = q;
    m_dirty |= DirtyFlags::View;
  }
  return *this;
}
PerspectiveCamera &PerspectiveCamera::fromTransform(const Transform &xf) {
  return setPosition(xf.getPosition()).setOrientation(xf.getOrientation());
}

bool PerspectiveCamera::isFrustumFreezed() const { return m_freezeFrustum; }

float PerspectiveCamera::getFov() const { return m_fov; }
float PerspectiveCamera::getAspectRatio() const { return m_aspectRatio; }
ClippingPlanes PerspectiveCamera::getClippingPlanes() const {
  return m_clippingPlanes;
}

glm::vec3 PerspectiveCamera::getPosition() const { return m_position; }
glm::quat PerspectiveCamera::getOrientation() const { return m_orientation; }

float PerspectiveCamera::getPitch() const { return glm::pitch(m_orientation); }
float PerspectiveCamera::getYaw() const { return glm::yaw(m_orientation); }
float PerspectiveCamera::getRoll() const { return glm::roll(m_orientation); }

glm::vec3 PerspectiveCamera::getRight() const {
  return m_orientation * Transform::kRight;
}
glm::vec3 PerspectiveCamera::getUp() const {
  return m_orientation * Transform::kUp;
}
glm::vec3 PerspectiveCamera::getForward() const {
  return m_orientation * Transform::kForward;
}

const glm::mat4 &PerspectiveCamera::getView() const {
  _update();
  return m_view;
}
const glm::mat4 &PerspectiveCamera::getProjection() const {
  _update();
  return m_projection;
}
const glm::mat4 &PerspectiveCamera::getViewProjection() const {
  _update();
  return m_viewProjection;
}

const Frustum &PerspectiveCamera::getFrustum() const {
  _update();
  return m_frustum;
}

//
// (private):
//

void PerspectiveCamera::_update() const {
  if (m_dirty == DirtyFlags::None) return;

  ZoneScopedN("PerspectiveCamera::Update");

  auto changed = false;
  if (bool(m_dirty & DirtyFlags::Projection)) {
    m_projection =
      glm::perspective(glm::radians(m_fov), m_aspectRatio,
                       m_clippingPlanes.zNear, m_clippingPlanes.zFar);

    m_dirty &= ~DirtyFlags::Projection;
    changed = true;
  }
  if (bool(m_dirty & DirtyFlags::View)) {
    m_view = glm::lookAt(m_position, m_position + getForward(), Transform::kUp);
    m_dirty &= ~DirtyFlags::View;
    changed = true;
  }

  if (changed) m_viewProjection = m_projection * m_view;

  if ((changed && !m_freezeFrustum) ||
      bool(m_dirty & DirtyFlags::FrustumUnfreezed)) {
    m_frustum.update(m_viewProjection);
    m_dirty &= ~DirtyFlags::FrustumUnfreezed;
  }
}

} // namespace gfx
