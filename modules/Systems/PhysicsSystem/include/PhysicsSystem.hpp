#pragma once

#include "SystemCommons.hpp"

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"
#include "physics/RigidBody.hpp"
#include "physics/Character.hpp"

/*
  Context variables:
  - [creates] PhysicsWorld
  Components:
  - Transform
  - [setup callbacks] ColliderComponent
  - [setup callbacks] RigidBody
  - [setup callbacks] Character
*/
struct PhysicsSystem {
  INTRODUCE_COMPONENTS(ColliderComponent, RigidBody, Character)

  static void setup(entt::registry &);

  // Simulate and update global Transform (Jolt -> Transform).
  static void simulate(entt::registry &, float timeStep);
  static void debugDraw(entt::registry &, DebugDraw &);

  // Update position and rotation of a body (Transform -> Jolt).
  static void updateTransform(entt::registry &, entt::entity);

  template <class Archive> static void save(Archive &archive) {
    auto &[registry, snapshot] = cereal::get_user_data<OutputContext>(archive);
    auto &ctx = registry.ctx();

    archive(ctx.get<PhysicsWorld>());

    snapshot.get<ColliderComponent>(archive)
      .get<RigidBody>(archive)
      .get<Character>(archive);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[registry, snapshotLoader] =
      cereal::get_user_data<InputContext>(archive);
    auto &ctx = registry.ctx();

    archive(ctx.get<PhysicsWorld>());

    snapshotLoader.get<ColliderComponent>(archive)
      .get<RigidBody>(archive)
      .get<Character>(archive);
  }

private:
  static void _connectRigidBodyComponent(entt::registry &);
  static void _initRigidBody(entt::registry &, entt::entity);
  static void _cleanupRigidBody(entt::registry &, entt::entity);

  static void _connectCharacterComponent(entt::registry &);
  static void _initCharacter(entt::registry &, entt::entity);
  static void _cleanupCharacter(entt::registry &, entt::entity);
};

[[nodiscard]] PhysicsWorld &getPhysicsWorld(entt::registry &);
