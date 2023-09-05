#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Character/Character.h"

#include "CollisionLayer.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

class Character {
  friend class PhysicsWorld;

public:
  struct Settings {
    float maxSlopeAngle{50.0f}; // In degrees.
    CollisionLayer layer;
    float mass{80.0f};

    float friction{0.2f};
    float gravityFactor{1.0f};

    template <class Archive> void serialize(Archive &archive) {
      archive(maxSlopeAngle, layer, mass, friction, gravityFactor);
    }
  };

  explicit Character(const Settings & = {});

  explicit operator bool() const;

  [[nodiscard]] const Settings &getSettings() const;

  JPH::BodyID getBodyId() const;

  void setRotation(const glm::quat &);
  void setLinearVelocity(const glm::vec3 &);

  [[nodiscard]] glm::vec3 getPosition() const;
  [[nodiscard]] glm::quat getRotation() const;
  [[nodiscard]] glm::vec3 getLinearVelocity() const;

  // ---

  [[nodiscard]] JPH::Character::EGroundState getGroundState() const;
  [[nodiscard]] bool isSupported() const;
  [[nodiscard]] glm::vec3 getGroundNormal() const;
  [[nodiscard]] glm::vec3 getGroundVelocity() const;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(m_settings);
  }

private:
  Settings m_settings{};
  JPH::Ref<JPH::Character> m_character{};
};

static_assert(std::is_copy_constructible_v<Character>);
