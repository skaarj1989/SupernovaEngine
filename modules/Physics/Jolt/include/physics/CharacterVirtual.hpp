#pragma once

#include "entt/signal/emitter.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"

#include "CollisionLayer.hpp"
#include "Events.hpp"

#include "glm/ext/quaternion_float.hpp"

class CharacterVirtual : private entt::emitter<CharacterVirtual> {
  friend class PhysicsWorld;
  friend class entt::emitter<CharacterVirtual>;

public:
  struct Settings {
    float maxSlopeAngle{50.0f}; // In degrees.
    CollisionLayer layer;
    float mass{70.0f};

    float maxStrength{100.0f};
    glm::vec3 shapeOffset{0.0f};
    JPH::EBackFaceMode backFaceMode{JPH::EBackFaceMode::CollideWithBackFaces};
    float predictiveContactDistance{0.1f};
    uint32_t maxCollisionIterations{5};
    uint32_t maxConstraintIterations{15};
    float minTimeRemaining{1.0e-4f};
    float collisionTolerance{1.0e-3f};
    float characterPadding{0.02f};
    uint32_t maxNumHits{256};
    float hitReductionCosMaxAngle{0.999f};
    float penetrationRecoverySpeed{1.0f};

    template <class Archive> void serialize(Archive &archive) {
      archive(maxSlopeAngle, layer, mass);
      archive(maxStrength, shapeOffset, backFaceMode, predictiveContactDistance,
              maxCollisionIterations, maxConstraintIterations, minTimeRemaining,
              collisionTolerance, characterPadding, maxNumHits,
              hitReductionCosMaxAngle, penetrationRecoverySpeed);
    }
  };

#ifdef __GNUG__
  static constexpr Settings defaultSettings() { return Settings{}; }
  explicit CharacterVirtual(const Settings & = defaultSettings());
#else
  explicit CharacterVirtual(const Settings & = {});
#endif
  CharacterVirtual(const CharacterVirtual &);

  using entt::emitter<CharacterVirtual>::emitter;

  using entt::emitter<CharacterVirtual>::publish;
  using entt::emitter<CharacterVirtual>::on;
  using entt::emitter<CharacterVirtual>::erase;

  explicit operator bool() const;

  [[nodiscard]] const Settings &getSettings() const;

  void setRotation(const glm::quat &);
  void setLinearVelocity(const glm::vec3 &);

  void setStickToFloor(bool);
  void setWalkStairs(bool);

  [[nodiscard]] glm::vec3 getUp() const;

  [[nodiscard]] glm::vec3 getPosition() const;
  [[nodiscard]] glm::quat getRotation() const;
  [[nodiscard]] glm::vec3 getLinearVelocity() const;

  [[nodiscard]] bool isStickToFloor() const;
  [[nodiscard]] bool canWalkStairs() const;

  // ---

  [[nodiscard]] JPH::CharacterVirtual::EGroundState getGroundState() const;
  [[nodiscard]] bool isSupported() const;
  [[nodiscard]] glm::vec3 getGroundNormal() const;
  [[nodiscard]] glm::vec3 getGroundVelocity() const;

  template <class Archive> void serialize(Archive &archive) {
    archive(m_settings, m_stickToFloor, m_walkStairs);
  }

private:
  Settings m_settings;
  JPH::Ref<JPH::CharacterVirtual> m_character;

  bool m_stickToFloor{true};
  bool m_walkStairs{true};
};

static_assert(std::is_copy_constructible_v<CharacterVirtual>);

template <> struct entt::type_hash<CharacterVirtual> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3267831443;
  }
};
