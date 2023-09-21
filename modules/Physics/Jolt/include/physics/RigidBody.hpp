#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"

#include "CollisionLayer.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

enum class MotionType { Dynamic, Static, Kinematic };

class RigidBody {
  friend class PhysicsWorld;
  static constexpr auto in_place_delete = true;

public:
  struct Settings {
    CollisionLayer layer;
    float mass{1.0f};
    MotionType motionType{MotionType::Static};
    float friction{0.2f};
    float restitution{0.0f};
    float linearDamping{0.05f};
    float angularDamping{0.05f};
    float gravityFactor{1.0f};

    template <class Archive> void serialize(Archive &archive) {
      archive(layer, mass, motionType, friction, restitution, linearDamping,
              angularDamping, gravityFactor);
    }
  };

  explicit RigidBody(const Settings & = {});

  explicit operator bool() const;

  const Settings &getSettings() const;

  JPH::BodyID getBodyId() const;

  void setPosition(const glm::vec3 &);
  void setRotation(const glm::quat &);
  void setLinearVelocity(const glm::vec3 &);

  void applyImpulse(const glm::vec3 &);

  [[nodiscard]] glm::vec3 getPosition() const;
  [[nodiscard]] glm::quat getRotation() const;
  [[nodiscard]] glm::vec3 getLinearVelocity() const;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(m_settings);
  }

private:
  JPH::PhysicsSystem *m_joltPhysics{nullptr};

  Settings m_settings{};
  JPH::BodyID m_bodyId{};
};

static_assert(std::is_copy_constructible_v<RigidBody>);
