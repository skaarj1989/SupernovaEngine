#pragma once

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"

#include "SystemCommons.hpp"

/*
  Context variables:
  - [creates] PhysicsWorld
  Components:
  - Transform
  - [setup callbacks] ColliderComponent
  - [setup callbacks] RigidBody
  - [setup callbacks] Character
  - [setup callbacks] CharacterVirtual
*/
struct PhysicsSystem {
  INTRODUCE_COMPONENTS(ColliderComponent, RigidBody, Character,
                       CharacterVirtual)

  static void setup(entt::registry &);

  // Simulate and update global Transform (Jolt -> Transform).
  static void simulate(entt::registry &, float timeStep);
  static void debugDraw(entt::registry &, DebugDraw &);

  // Update position and rotation of a body (Transform -> Jolt).
  static void updateTransform(entt::registry &, entt::entity);

  template <class Archive> static void save(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<OutputContext>(archive);
    archive(registry.ctx().get<PhysicsWorld>());
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<InputContext>(archive);
    archive(registry.ctx().get<PhysicsWorld>());
  }
};

[[nodiscard]] PhysicsWorld &getPhysicsWorld(entt::registry &);
