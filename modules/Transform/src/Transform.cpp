#include "Transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"        // translate, scale
#include "glm/gtx/quaternion.hpp"       // toMat4
#include "glm/gtx/matrix_decompose.hpp" // decompose

Transform::Transform(const glm::vec3 &position, const glm::quat &orientation,
                     const glm::vec3 &scale)
    : m_position{position}, m_orientation{orientation}, m_scale{scale},
      m_dirty{true} {}
Transform::Transform(const glm::mat4 &m) { load(m); }
Transform::Transform(const Transform &other)
    : m_position{other.m_position}, m_orientation{other.m_orientation},
      m_scale{other.m_scale}, m_dirty{true} {}

Transform &Transform::operator=(const Transform &rhs) {
  m_position = rhs.m_position;
  m_orientation = rhs.m_orientation;
  m_scale = rhs.m_scale;
  m_dirty = true;
  return *this;
}

Transform &Transform::setParent(const Transform *const xf) {
  if (this != xf) m_parent = xf;
  return *this;
}

Transform &Transform::load(const glm::mat4 &m) {
  // Don't care about 'skew' and 'perspective'.
  static glm::vec3 _0;
  static glm::vec4 _1;
  glm::decompose(m, m_scale, m_orientation, m_position, _0, _1);

  m_modelMatrix = m;
  m_dirty = false;
  return *this;
}
Transform &Transform::loadIdentity() {
  m_position = glm::vec3{0.0f};
  m_orientation = glm::identity<glm::quat>();
  m_scale = glm::vec3{1.0f};

  m_modelMatrix = glm::identity<glm::mat4>();
  m_dirty = false;
  return *this;
}

Transform &Transform::setPosition(const glm::vec3 &v) {
  if (m_position != v) {
    m_position = v;
    m_dirty = true;
  }
  return *this;
}
Transform &Transform::setOrientation(const glm::quat &q) {
  m_orientation = glm::normalize(q);
  m_dirty = true;
  return *this;
}
Transform &Transform::setEulerAngles(const glm::vec3 &eulerAngles) {
  return setOrientation(glm::quat{eulerAngles});
}
Transform &Transform::setScale(const glm::vec3 &v) {
  if (m_scale != v) {
    m_scale = v;
    m_dirty = true;
  }
  return *this;
}

bool Transform::isChildOf(const Transform &other) const {
  return m_parent == &other;
}
const Transform *Transform::getParent() const { return m_parent; }

const glm::mat4 &Transform::getModelMatrix() const {
  if (m_dirty) {
    m_modelMatrix = glm::translate(m_position) * glm::toMat4(m_orientation) *
                    glm::scale(m_scale);
    m_dirty = false;
  }
  return m_modelMatrix;
}
glm::mat4 Transform::getWorldMatrix() const {
  return m_parent ? m_parent->getWorldMatrix() * getModelMatrix()
                  : getModelMatrix();
}

glm::vec3 Transform::getLocalPosition() const { return m_position; }
glm::quat Transform::getLocalOrientation() const { return m_orientation; }
glm::vec3 Transform::getLocalEulerAngles() const {
  return glm::eulerAngles(getLocalOrientation());
}
glm::vec3 Transform::getLocalScale() const { return m_scale; }

glm::vec3 Transform::getPosition() const {
  return m_parent ? m_parent->getPosition() + m_position : getLocalPosition();
}
glm::quat Transform::getOrientation() const {
  return m_parent
           ? glm::normalize(m_parent->getOrientation() * getLocalOrientation())
           : getLocalOrientation();
}
glm::vec3 Transform::getEulerAngles() const {
  return glm::eulerAngles(getOrientation());
}
glm::vec3 Transform::getScale() const {
  return m_parent ? m_parent->getScale() * getLocalScale() : getLocalScale();
}

glm::vec3 Transform::getRight() const {
  return glm::normalize(getWorldMatrix()[0]);
}
glm::vec3 Transform::getUp() const {
  return glm::normalize(getWorldMatrix()[1]);
}
glm::vec3 Transform::getForward() const {
  return glm::normalize(getWorldMatrix()[2]);
}

Transform &Transform::translate(const glm::vec3 &v) {
  m_position += v;
  m_dirty = true;
  return *this;
}

Transform &Transform::rotate(const glm::quat &q) {
  m_orientation = glm::normalize(m_orientation * q);
  m_dirty = true;
  return *this;
}
Transform &Transform::pitch(float angle) {
  return rotate(glm::quat{glm::vec3{angle, 0.0f, 0.0f}});
}
Transform &Transform::yaw(float angle) {
  return rotate(glm::quat{glm::vec3{0.0f, angle, 0.0f}});
}
Transform &Transform::roll(float angle) {
  return rotate(glm::quat{glm::vec3{0.0f, 0.0f, angle}});
}

Transform &Transform::lookAt(const glm::vec4 &target) {
  const auto direction =
    target.w == 0 ? glm::vec3{target} : glm::vec3{target} - m_position;
  return setOrientation(glm::quatLookAt(-glm::normalize(direction), kUp));
}
Transform &Transform::lookAt(const Transform &target) {
  return lookAt(glm::vec4{target.getPosition(), 1.0f});
}

Transform &Transform::scale(const glm::vec3 &v) {
  m_scale *= v;
  m_dirty = true;
  return *this;
}

//
// Helper:
//

glm::vec3 calculateUpVector(const glm::vec3 &v) {
  const auto forward = glm::normalize(v);
  // https://math.stackexchange.com/questions/23259/is-the-cross-product-of-two-unit-vectors-itself-a-unit-vector
  const auto right = glm::normalize(glm::cross(forward, Transform::kUp));
  return glm::normalize(glm::cross(right, forward));
}
