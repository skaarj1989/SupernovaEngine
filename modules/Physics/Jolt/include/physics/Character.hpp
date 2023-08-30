#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Character/Character.h"

#include "physics/CharacterSettings.hpp"

#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

class Character {
  friend class PhysicsSystem;

public:
  explicit Character(const CharacterSettings & = {});

  void setCollisionShape(const JPH::Shape *);

  const CharacterSettings &getSettings() const;

  [[nodiscard]] glm::quat getRotation() const;
  void setRotation(const glm::quat &);

  void setLinearVelocity(const glm::vec3 &);
  [[nodiscard]] glm::vec3 getLinearVelocity() const;

  [[nodiscard]] JPH::Character::EGroundState getGroundState() const;
  [[nodiscard]] glm::vec3 getGroundNormal() const;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(m_settings);
  }

private:
  CharacterSettings m_settings{};
  JPH::Ref<JPH::Character> m_character{};
};

static_assert(std::is_copy_constructible_v<Character>);
