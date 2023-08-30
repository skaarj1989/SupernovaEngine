#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "physics/RigidBodySettings.hpp"

#include "glm/vec3.hpp"

class RigidBody {
  friend class PhysicsSystem;
  static constexpr auto in_place_delete = true;

public:
  explicit RigidBody(const RigidBodySettings & = {});

  void setCollisionShape(const JPH::Shape *);

  const RigidBodySettings &getSettings() const;

  glm::vec3 getLinearVelocity() const;
  void setLinearVelocity(const glm::vec3 &);

  void applyImpulse(const glm::vec3 &);

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(m_settings);
  }

private:
  JPH::PhysicsSystem *m_joltPhysics{nullptr};

  RigidBodySettings m_settings{};
  JPH::BodyID m_bodyId{};
};

static_assert(std::is_copy_constructible_v<RigidBody>);
