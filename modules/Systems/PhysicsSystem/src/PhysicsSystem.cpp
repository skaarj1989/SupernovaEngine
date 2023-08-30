#include "PhysicsSystem.hpp"

#include "physics/Conversion.hpp"
#include "physics/Collider.hpp"
#include "physics/RigidBody.hpp"
#include "physics/Character.hpp"

#include "spdlog/spdlog.h"

namespace {

void setTransform(const JPH::RMat44 &src, Transform &dst) {
  dst.setPosition(to_glm(src.GetTranslation()))
    .setOrientation(to_glm(src.GetRotation()));
}
void setTransform(JPH::BodyInterface &bodyInterface, const JPH::BodyID bodyId,
                  const Transform &xf) {
  bodyInterface.SetPositionAndRotation(bodyId, to_Jolt(xf.getPosition()),
                                       to_Jolt(xf.getOrientation()),
                                       JPH::EActivation::DontActivate);
}

void setUserData(const JPH::BodyLockInterfaceLocking &lockInterface,
                 JPH::BodyID bodyId, entt::entity e) {
  assert(!bodyId.IsInvalid());

  JPH::BodyLockWrite lock{lockInterface, bodyId};
  assert(lock.Succeeded());
  lock.GetBody().SetUserData(entt::to_integral(e));
}

[[nodiscard]] auto *getCollisionShape(entt::registry &r, entt::entity e) {
  const auto *c = r.try_get<ColliderComponent>(e);
  return (c && c->resource) ? c->resource->getCollisionShape() : nullptr;
}

//
// ColliderComponent:
//

void initCollider(entt::registry &r, entt::entity e) {
  if (const auto &[colliderResource] = r.get<ColliderComponent>(e);
      colliderResource) {
    if (auto *c = r.try_get<RigidBody>(e); c)
      c->setCollisionShape(colliderResource->getCollisionShape());
  }
}
void updateCollider(entt::registry &r, entt::entity e) { initCollider(r, e); }
void cleanupCollider(entt::registry &r, entt::entity e) {
  r.remove<RigidBody>(e);
}

void connectColliderComponent(entt::registry &r) {
  r.on_construct<ColliderComponent>().connect<&initCollider>();
  r.on_update<ColliderComponent>().connect<&updateCollider>();
  r.on_destroy<ColliderComponent>().connect<&cleanupCollider>();
}

} // namespace

void PhysicsSystem::setup(entt::registry &r) {
  r.ctx().emplace<PhysicsWorld>();
  connectColliderComponent(r);
  _connectRigidBodyComponent(r);
  _connectCharacterComponent(r);
}

void PhysicsSystem::simulate(entt::registry &r, float timeStep) {
  auto &world = getPhysicsWorld(r);
  world.simulate(timeStep);

  const auto transforms = r.view<Transform>();

  const auto &bodyInterface = world.getBodyInterface();
  for (auto [_, xf, rigidBody] :
       (transforms | r.view<const RigidBody>()).each()) {
    if (!rigidBody.m_bodyId.IsInvalid()) {
      setTransform(bodyInterface.GetWorldTransform(rigidBody.m_bodyId), xf);
    }
  }
  for (auto [_, xf, character] :
       (transforms | r.view<const Character>()).each()) {
    constexpr auto kCollisionTolerance = 1.5f;
    if (character.m_character) {
      character.m_character->PostSimulation(kCollisionTolerance);
      setTransform(character.m_character->GetWorldTransform(), xf);
    }
  }
}
void PhysicsSystem::debugDraw(entt::registry &r, DebugDraw &dd) {
  getPhysicsWorld(r).debugDraw(dd);
}

void PhysicsSystem::updateTransform(entt::registry &r, entt::entity e) {
  if (const auto *xf = r.try_get<const Transform>(e); xf) {
    auto &bodyInterface = getPhysicsWorld(r).getBodyInterface();
    if (const auto *c = r.try_get<RigidBody>(e); c) {
      setTransform(bodyInterface, c->m_bodyId, *xf);
    }
    if (const auto *c = r.try_get<Character>(e); c) {
      setTransform(bodyInterface, c->m_character->GetBodyID(), *xf);
    }
  }
}

//
// (private):
//

void PhysicsSystem::_connectRigidBodyComponent(entt::registry &r) {
  r.on_construct<RigidBody>().connect<&PhysicsSystem::_initRigidBody>();
  r.on_destroy<RigidBody>().connect<&PhysicsSystem::_cleanupRigidBody>();
}
void PhysicsSystem::_initRigidBody(entt::registry &r, entt::entity e) {
  const auto *collisionShape = getCollisionShape(r, e);
  if (!collisionShape) {
    // Can't initialize a body without a shape.
    return;
  }

  auto &world = getPhysicsWorld(r);

  auto &c = r.get<RigidBody>(e);
  const auto &xf = r.get_or_emplace<Transform>(e);
  const auto *body = world.createBody(xf, c.m_settings, collisionShape);
  c.m_joltPhysics = std::addressof(world.getJolt());
  c.m_bodyId = body->GetID();
  setUserData(world.getBodyLockInterface(), c.m_bodyId, e);
}
void PhysicsSystem::_cleanupRigidBody(entt::registry &r, entt::entity e) {
  const auto &rigidBody = r.get<const RigidBody>(e);
  if (!rigidBody.m_bodyId.IsInvalid()) {
    getPhysicsWorld(r).destroyBody(rigidBody.m_bodyId);
  }
}

void PhysicsSystem::_connectCharacterComponent(entt::registry &r) {
  r.on_construct<Character>().connect<&PhysicsSystem::_initCharacter>();
  r.on_destroy<Character>().connect<&PhysicsSystem::_cleanupCharacter>();
}
void PhysicsSystem::_initCharacter(entt::registry &r, entt::entity e) {
  const auto *collisionShape = getCollisionShape(r, e);
  if (!collisionShape) {
    // Can't initialize a character without a shape.
    return;
  }

  auto &world = getPhysicsWorld(r);

  auto &c = r.get<Character>(e);
  const auto &xf = r.get_or_emplace<Transform>(e);
  auto character = world.createCharacter(xf, c.m_settings, collisionShape);
  setUserData(world.getBodyLockInterface(), character->GetBodyID(), e);

  c.m_character = character;
}
void PhysicsSystem::_cleanupCharacter(entt::registry &r, entt::entity e) {
  auto &c = r.get<const Character>(e);
  if (c.m_character) c.m_character->RemoveFromPhysicsSystem();
}

//
// Helper:
//

PhysicsWorld &getPhysicsWorld(entt::registry &r) {
  return r.ctx().get<PhysicsWorld>();
}
