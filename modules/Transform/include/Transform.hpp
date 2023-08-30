#pragma once

#include "glm/ext/quaternion_float.hpp"

class Transform {
  static constexpr auto in_place_delete = true;

public:
  explicit Transform(const glm::vec3 &position = glm::vec3{0.0f},
                     const glm::quat & = {1.0f, 0.0f, 0.0f, 0.0f},
                     const glm::vec3 &scale = glm::vec3{1.0f});
  explicit Transform(const glm::mat4 &);
  Transform(const Transform &);
  ~Transform() = default;

  Transform &operator=(const Transform &);

  // ---

  inline static const glm::vec3 kRight{1.0f, 0.0f, 0.0f};
  inline static const glm::vec3 kUp{0.0f, 1.0f, 0.0f};
  inline static const glm::vec3 kForward{0.0f, 0.0f, 1.0f};

  // ---

  Transform &setParent(const Transform *const);

  Transform &load(const glm::mat4 &);
  Transform &loadIdentity();

  Transform &setPosition(const glm::vec3 &);
  Transform &setOrientation(const glm::quat &);
  // @param eulerAngles In radians (x = pitch, y = yaw, z = roll).
  Transform &setEulerAngles(const glm::vec3 &eulerAngles);
  Transform &setScale(const glm::vec3 &);

  // ---

  [[nodiscard]] bool isChildOf(const Transform &) const;
  const Transform *getParent() const;

  [[nodiscard]] const glm::mat4 &getModelMatrix() const;
  // Contains parent transformation.
  glm::mat4 getWorldMatrix() const;

  // @returns Position relative to the parent transform.
  glm::vec3 getLocalPosition() const;
  // @returns Orientation relative to the parent transform.
  [[nodiscard]] glm::quat getLocalOrientation() const;
  // @returns Euler angles in radians (x = pitch, y = yaw, z = roll).
  // Relative to the parent transform.
  [[nodiscard]] glm::vec3 getLocalEulerAngles() const;
  // @returns Scale relative to the parent transform.
  [[nodiscard]] glm::vec3 getLocalScale() const;

  // @returns Position relative to the world origin.
  [[nodiscard]] glm::vec3 getPosition() const;
  // @returns Orientation relative to the world origin.
  [[nodiscard]] glm::quat getOrientation() const;
  // @returns Euler angles in radians (x = pitch, y = yaw, z = roll).
  // Relative to the world origin.
  [[nodiscard]] glm::vec3 getEulerAngles() const;
  // @returns Absolute scale.
  [[nodiscard]] glm::vec3 getScale() const;

  // @returns Normalized vector in world space (right = x axis).
  [[nodiscard]] glm::vec3 getRight() const;
  // @returns Normalized vector in world space (up = y axis).
  [[nodiscard]] glm::vec3 getUp() const;
  // @returns Normalized vector in world space (forward = z axis).
  [[nodiscard]] glm::vec3 getForward() const;

  // ---

  Transform &translate(const glm::vec3 &);

  Transform &rotate(const glm::quat &);
  Transform &pitch(float angle);
  Transform &yaw(float angle);
  Transform &roll(float angle);

  // Position (.w = 1).
  // Direction (.w = 0).
  Transform &lookAt(const glm::vec4 &);
  Transform &lookAt(const Transform &);

  Transform &scale(const glm::vec3 &);

  // ---

  template <class Archive> void save(Archive &archive) const {
    archive(m_position, m_orientation, m_scale);
  }
  template <class Archive> void load(Archive &archive) {
    archive(m_position, m_orientation, m_scale);
    m_dirty = true;
  }

private:
  const Transform *m_parent{nullptr};

  glm::vec3 m_position{0.0f};
  glm::quat m_orientation{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 m_scale{1.0f};

  mutable glm::mat4 m_modelMatrix{1.0f}; // Model-to-world.
  mutable bool m_dirty{false};
};

[[nodiscard]] glm::vec3 calculateUpVector(const glm::vec3 &);
