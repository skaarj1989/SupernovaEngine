#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Core/JobSystemThreadPool.h"

#include "physics/DebugRenderer.hpp"
#include "physics/RigidBody.hpp"
#include "physics/Character.hpp"
#include "physics/CharacterVirtual.hpp"
#include "Transform.hpp"

struct RayCastBPResult {
  glm::vec3 position;
  uint32_t entityId;
};
using RayCastBPResults = std::vector<RayCastBPResult>;

struct RayCastNPResult {
  glm::vec3 position;
  glm::vec3 normal;
  uint32_t entityId;
};

class PhysicsWorld {
public:
  PhysicsWorld();

  void enableDebugDraw(bool);
  bool isDebugDrawEnabled() const;

  void setGravity(const glm::vec3 &);
  [[nodiscard]] glm::vec3 getGravity() const;

  struct CreateInfo {
    const Transform &transform;
    const JPH::Shape *shape{nullptr};
    uint32_t userData{0};
  };
  void initBody(RigidBody &, const CreateInfo &);
  void initCharacter(Character &, const CreateInfo &);
  void initCharacter(CharacterVirtual &, const CreateInfo &);

  void remove(const RigidBody &);
  void remove(const Character &);

  void setCollisionShape(RigidBody &, const JPH::Shape *);
  void setCollisionShape(Character &, const JPH::Shape *);
  void setCollisionShape(CharacterVirtual &, const JPH::Shape *);

  // @note BroadPhase
  [[nodiscard]] RayCastBPResults castRayBP(const glm::vec3 &from,
                                           const glm::vec3 &direction);

  // @note NarrowPhase
  [[nodiscard]] std::optional<RayCastNPResult>
  castRayNP(const glm::vec3 &from, const glm::vec3 &direction);

  [[nodiscard]] JPH::BodyManager::BodyStats getBodyStats() const;

  auto &getBodyInterface() { return m_physicsSystem.GetBodyInterface(); }
  auto &getBodyLockInterface() const {
    return m_physicsSystem.GetBodyLockInterface();
  }

  void update(const Character &, Transform *, float collisionTollerance);
  void update(const CharacterVirtual &, Transform *, float timeStep);

  void simulate(float timeStep);
  void debugDraw(DebugDraw &);

  // ---

  template <class Archive> void save(Archive &archive) const {
    archive(getGravity());
  }
  template <class Archive> void load(Archive &archive) {
    glm::vec3 v;
    archive(v);
    setGravity(v);
  }

private:
  void _destroy(const JPH::BodyID);

  [[nodiscard]] JPH::BodyID _createBody(const RigidBody::Settings &,
                                        const CreateInfo &);
  [[nodiscard]] JPH::Ref<JPH::Character>
  _createCharacter(const Character::Settings &, const CreateInfo &);
  [[nodiscard]] JPH::Ref<JPH::CharacterVirtual>
  _createCharacter(const CharacterVirtual::Settings &settings,
                   const CreateInfo &);

private:
  JPH::PhysicsSystem m_physicsSystem;

  std::unique_ptr<JPH::TempAllocator> m_tempAllocator;
  std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
  std::unique_ptr<JPH::BroadPhaseLayerInterface> m_broadPhaseLayerInterface;
  std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter>
    m_objectVsBroadPhaseLayerFilter;
  std::unique_ptr<JPH::ObjectLayerPairFilter> m_objectVsObjectLayerFilter;

  bool m_debugDrawEnabled{false};
};

// Jolt -> Engine
void setTransform(const JPH::RMat44 &src, Transform &dst);
