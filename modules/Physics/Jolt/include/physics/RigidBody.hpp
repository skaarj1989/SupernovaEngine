#pragma once

#include "entt/signal/emitter.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include "CollisionLayer.hpp"
#include "Events.hpp"

#include "glm/ext/quaternion_float.hpp"

enum class MotionType { Dynamic, Static, Kinematic };

class RigidBody : private entt::emitter<RigidBody> {
  friend class PhysicsWorld;
  friend class entt::emitter<RigidBody>;

  static constexpr auto in_place_delete = true;

public:
  struct Settings {
    CollisionLayer layer;
    bool isSensor{false};
    float mass{1.0f};
    MotionType motionType{MotionType::Static};
    float friction{0.2f};
    float restitution{0.0f};
    float linearDamping{0.05f};
    float angularDamping{0.05f};
    float gravityFactor{1.0f};

    template <class Archive> void serialize(Archive &archive) {
      archive(layer, isSensor, mass, motionType, friction, restitution,
              linearDamping, angularDamping, gravityFactor);
    }
  };

#ifdef __GNUG__
  static constexpr Settings defaultSettings() { return Settings{}; }
  explicit RigidBody(const Settings & = defaultSettings());
#else
  explicit RigidBody(const Settings & = {});
#endif

  RigidBody(const RigidBody &);

  using entt::emitter<RigidBody>::emitter;

  using entt::emitter<RigidBody>::publish;
  using entt::emitter<RigidBody>::on;
  using entt::emitter<RigidBody>::erase;

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

template <> struct entt::type_hash<RigidBody> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 1548160800;
  }
};
