#pragma once

#include "Jolt/Jolt.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Core/JobSystemThreadPool.h"

#include "ScopedEnumFlags.hpp"

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

using BodyUserDataPair = std::pair<uint32_t, uint32_t>;

struct ContactAddedEvent {
  BodyUserDataPair bodyUserDataPair;
  glm::vec3 offset;
  glm::vec3 normal;
};
struct ContactRemovedEvent {
  BodyUserDataPair bodyUserDataPair;
};

class PhysicsWorld : private entt::emitter<PhysicsWorld>,
                     private JPH::ContactListener,
                     private JPH::CharacterContactListener {
  friend class entt::emitter<PhysicsWorld>;

public:
  PhysicsWorld();

  using entt::emitter<PhysicsWorld>::on;
  using entt::emitter<PhysicsWorld>::erase;

  enum class DebugDrawFlags {
    None = 0,
    Shape = 1 << 0,
    BoundingBox = 1 << 1,
    WorldTransform = 1 << 2,
  };

  void setDebugDrawFlags(const DebugDrawFlags);
  DebugDrawFlags getDebugDrawFlags() const;

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
  void OnContactAdded(const JPH::Body &body1, const JPH::Body &body2,
                      const JPH::ContactManifold &,
                      JPH::ContactSettings &) override;
  void OnContactRemoved(const JPH::SubShapeIDPair &) override;

  void OnContactAdded(const JPH::CharacterVirtual *, const JPH::BodyID &body2,
                      const JPH::SubShapeID &, JPH::RVec3Arg position,
                      JPH::Vec3Arg normal,
                      JPH::CharacterContactSettings &) override;

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

  DebugDrawFlags m_debugDrawFlags{DebugDrawFlags::None};
};
template <> struct has_flags<PhysicsWorld::DebugDrawFlags> : std::true_type {};

// Jolt -> Engine
void setTransform(const JPH::RMat44 &src, Transform &dst);
