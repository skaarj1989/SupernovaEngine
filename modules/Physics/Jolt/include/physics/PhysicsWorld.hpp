#pragma once

#include "Transform.hpp"
#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/Character/Character.h"

#include "physics/JoltPhysics.hpp"
#include "physics/DebugRenderer.hpp"
#include "physics/RigidBodySettings.hpp"
#include "physics/CharacterSettings.hpp"

class PhysicsWorld {
public:
  PhysicsWorld();

  void enableDebugDraw(bool);
  bool isDebugDrawEnabled() const;

  void setGravity(const glm::vec3 &);
  [[nodiscard]] glm::vec3 getGravity() const;

  [[nodiscard]] JPH::Body *
  createBody(const Transform &, const RigidBodySettings &, const JPH::Shape *);
  void destroyBody(JPH::BodyID);

  [[nodiscard]] JPH::Ref<JPH::Character>
  createCharacter(const Transform &, const CharacterSettings &,
                  const JPH::Shape *);

  auto &getJolt() { return m_physicsSystem; }

  auto &getBodyInterface() { return m_physicsSystem.GetBodyInterface(); }
  auto &getBodyLockInterface() const {
    return m_physicsSystem.GetBodyLockInterface();
  }

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
  JPH::PhysicsSystem m_physicsSystem;

  std::unique_ptr<JPH::TempAllocator> m_tempAllocator;
  std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
  std::unique_ptr<JPH::BroadPhaseLayerInterface> m_broadPhaseLayerInterface;
  std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter>
    m_objectVsBroadPhaseLayerFilter;
  std::unique_ptr<JPH::ObjectLayerPairFilter> m_objectVsObjectLayerFilter;

  bool m_debugDrawEnabled{false};
};
