#include "PhysicsSystem.hpp"
#include "physics/Conversion.hpp"

namespace {

// Engine -> Jolt
void setTransform(JPH::BodyInterface &bodyInterface, const JPH::BodyID bodyId,
                  const Transform &xf) {
  bodyInterface.SetPositionAndRotation(bodyId, to_Jolt(xf.getPosition()),
                                       to_Jolt(xf.getOrientation()),
                                       JPH::EActivation::DontActivate);
}

[[nodiscard]] auto *getCollisionShape(entt::registry &r, entt::entity e) {
  const auto *c = r.try_get<ColliderComponent>(e);
  return (c && c->resource) ? c->resource->getCollisionShape() : nullptr;
}

//
// ColliderComponent:
//

void initCollider(entt::registry &r, entt::entity e) {
  if (const auto *collisionShape = getCollisionShape(r, e); collisionShape) {
    auto &world = getPhysicsWorld(r);
    if (auto *rb = r.try_get<RigidBody>(e); rb) {
      world.setCollisionShape(*rb, collisionShape);
    }
    if (auto *c = r.try_get<Character>(e); c) {
      world.setCollisionShape(*c, collisionShape);
    }
    if (auto *c = r.try_get<CharacterVirtual>(e); c) {
      world.setCollisionShape(*c, collisionShape);
    }
  }
}
void updateCollider(entt::registry &r, entt::entity e) { initCollider(r, e); }
void cleanupCollider(entt::registry &r, entt::entity e) {
  r.remove<RigidBody, Character, CharacterVirtual>(e);
}
void connectColliderComponent(entt::registry &r) {
  r.on_construct<ColliderComponent>().connect<&initCollider>();
  r.on_update<ColliderComponent>().connect<&updateCollider>();
  r.on_destroy<ColliderComponent>().connect<&cleanupCollider>();
}

//
// RigidBody:
//

void initRigidBody(entt::registry &r, entt::entity e) {
  if (const auto *collisionShape = getCollisionShape(r, e); collisionShape) {
    auto &rb = r.get<RigidBody>(e);
    const auto &xf = r.get_or_emplace<Transform>(e);
    getPhysicsWorld(r).initBody(rb, {
                                      .transform = xf,
                                      .shape = collisionShape,
                                      .userData = entt::to_integral(e),
                                    });
  }
}
void cleanupRigidBody(entt::registry &r, entt::entity e) {
  const auto &rb = r.get<const RigidBody>(e);
  getPhysicsWorld(r).remove(rb);
}
void connectRigidBodyComponent(entt::registry &r) {
  r.on_construct<RigidBody>().connect<&initRigidBody>();
  r.on_destroy<RigidBody>().connect<&cleanupRigidBody>();
}

//
// Character:
//

void initCharacter(entt::registry &r, entt::entity e) {
  if (const auto *collisionShape = getCollisionShape(r, e); collisionShape) {
    auto &c = r.get<Character>(e);
    const auto &xf = r.get_or_emplace<Transform>(e);
    getPhysicsWorld(r).initCharacter(c, {
                                          .transform = xf,
                                          .shape = collisionShape,
                                          .userData = entt::to_integral(e),
                                        });
  }
}
void cleanupCharacter(entt::registry &r, entt::entity e) {
  auto &c = r.get<const Character>(e);
  getPhysicsWorld(r).remove(c);
}
void connectCharacterComponent(entt::registry &r) {
  r.on_construct<Character>().connect<&initCharacter>();
  r.on_destroy<Character>().connect<&cleanupCharacter>();
}

//
// CharacterVirtual:
//

void initCharacterVirtual(entt::registry &r, entt::entity e) {
  if (const auto *collisionShape = getCollisionShape(r, e); collisionShape) {
    auto &c = r.get<CharacterVirtual>(e);
    const auto &xf = r.get_or_emplace<Transform>(e);
    getPhysicsWorld(r).initCharacter(c, {
                                          .transform = xf,
                                          .shape = collisionShape,
                                        });
  }
}
void connectCharacterVirtualComponent(entt::registry &r) {
  r.on_construct<CharacterVirtual>().connect<&initCharacterVirtual>();
}

} // namespace

void PhysicsSystem::setup(entt::registry &r) {
  r.ctx().emplace<PhysicsWorld>();
  connectColliderComponent(r);
  connectRigidBodyComponent(r);
  connectCharacterComponent(r);
  connectCharacterVirtualComponent(r);
}

void PhysicsSystem::simulate(entt::registry &r, float timeStep) {
  auto &world = getPhysicsWorld(r);
  world.simulate(timeStep);

  const auto transforms = r.view<Transform>();

  const auto &bodyInterface = world.getBodyInterface();
  for (auto [_, xf, rb] : (transforms | r.view<const RigidBody>()).each()) {
    if (rb) setTransform(bodyInterface.GetWorldTransform(rb.getBodyId()), xf);
  }
  for (auto [_, xf, c] : (transforms | r.view<const Character>()).each()) {
    constexpr auto kCollisionTolerance = 0.05f;
    world.update(c, &xf, kCollisionTolerance);
  }
  for (auto [_, xf, c] :
       (transforms | r.view<const CharacterVirtual>()).each()) {
    world.update(c, &xf, timeStep);
  }
}
void PhysicsSystem::debugDraw(entt::registry &r, DebugDraw &dd) {
  getPhysicsWorld(r).debugDraw(dd);
}

void PhysicsSystem::updateTransform(entt::registry &r, entt::entity e) {
  if (const auto *xf = r.try_get<const Transform>(e); xf) {
    auto &bodyInterface = getPhysicsWorld(r).getBodyInterface();
    if (const auto *rb = r.try_get<RigidBody>(e); rb) {
      setTransform(bodyInterface, rb->getBodyId(), *xf);
    }
    if (const auto *c = r.try_get<Character>(e); c) {
      setTransform(bodyInterface, c->getBodyId(), *xf);
    }
  }
}

//
// Helper:
//

PhysicsWorld &getPhysicsWorld(entt::registry &r) {
  return r.ctx().get<PhysicsWorld>();
}
