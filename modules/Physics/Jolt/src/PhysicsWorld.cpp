#include "Jolt/Jolt.h"

#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"

#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Character/Character.h"

#include "physics/PhysicsWorld.hpp"
#include "physics/Conversion.hpp"

#include <cassert>

namespace {

[[nodiscard]] auto convert(MotionType motionType) {
#define CASE(Value)                                                            \
  case MotionType::Value:                                                      \
    return JPH::EMotionType::Value

  switch (motionType) {
    CASE(Dynamic);
    CASE(Static);
    CASE(Kinematic);
  }
#undef CASE

  assert(false);
  return JPH::EMotionType::Static;
}

[[nodiscard]] auto encode(const CollisionLayer &layer) {
  return JPH::ObjectLayer{
    static_cast<uint16_t>(layer.group | (layer.mask << 8))};
}
[[nodiscard]] auto decode(JPH::ObjectLayer u16) {
  return CollisionLayer{
    .group = uint8_t(u16 & 0xFF),
    .mask = uint8_t((u16 >> 8) & 0xFF),
  };
}

[[nodiscard]] auto makeSettings(const Transform &xf,
                                const RigidBodySettings &settings,
                                const JPH::Shape *shape) {
  assert(shape);

  JPH::BodyCreationSettings out{
    shape,
    to_Jolt(xf.getPosition()),
    to_Jolt(xf.getOrientation()),
    convert(settings.motionType),
    encode(settings.layer),
  };
  out.mAllowDynamicOrKinematic = true;
  out.mFriction = settings.friction;
  out.mRestitution = settings.restitution;
  out.mLinearDamping = settings.linearDamping;
  out.mAngularDamping = settings.angularDamping;
  out.mGravityFactor = settings.gravityFactor;

  if (shape->MustBeStatic()) {
    out.mAllowDynamicOrKinematic = false;
    out.mOverrideMassProperties =
      JPH::EOverrideMassProperties::MassAndInertiaProvided;
  }

  return out;
}

[[nodiscard]] auto makeSettings(const CharacterSettings &settings,
                                const JPH::Shape *shape) {
  assert(shape);

  JPH::CharacterSettings out;
  out.mMaxSlopeAngle = JPH::DegreesToRadians(settings.maxSlopeAngle);
  out.mShape = shape;
  out.mLayer = encode(settings.layer);
  out.mMass = settings.mass;
  out.mFriction = settings.friction;
  out.mGravityFactor = settings.gravityFactor;
  return out;
}

class BPLayerInterface final : public JPH::BroadPhaseLayerInterface {
public:
  BPLayerInterface() = default;

  uint32_t GetNumBroadPhaseLayers() const override { return 1; }

  JPH::BroadPhaseLayer
  GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
    return JPH::BroadPhaseLayer{};
  }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
  const char *
  GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
    return "Default";
  }
#endif
};

class ObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
public:
  bool ShouldCollide(JPH::ObjectLayer object1,
                     JPH::ObjectLayer object2) const override {
    const auto a = decode(object1);
    const auto b = decode(object2);
    return ((1 << a.group) & b.mask) || ((1 << b.group) & a.mask);
  }
};

class ContactListener final : public JPH::ContactListener {
public:
  void OnContactAdded(const JPH::Body &body1, const JPH::Body &body2,
                      const JPH::ContactManifold &manifold,
                      JPH::ContactSettings &settings) override {}
  void OnContactRemoved(const JPH::SubShapeIDPair &subShapePair) override {}
};

} // namespace

//
// PhysicsWorld class:
//

PhysicsWorld::PhysicsWorld() {
  // This is the max amount of rigid bodies that you can add to the physics
  // system. If you try to add more you'll get an error. Note: This value is low
  // because this is a simple test. For a real project use something in the
  // order of 65536.
  constexpr auto kMaxBodies = 65536;

  // This determines how many mutexes to allocate to protect rigid bodies from
  // concurrent access. Set it to 0 for the default settings.
  constexpr auto kNumBodyMutexes = 0;

  // This is the max amount of body pairs that can be queued at any time (the
  // broad phase will detect overlapping body pairs based on their bounding
  // boxes and will insert them into a queue for the narrowphase). If you make
  // this buffer too small the queue will fill up and the broad phase jobs will
  // start to do narrow phase work. This is slightly less efficient. Note: This
  // value is low because this is a simple test. For a real project use
  // something in the order of 65536.
  constexpr auto kMaxBodyPairs = 65536;

  // This is the maximum size of the contact constraint buffer. If more contacts
  // (collisions between bodies) are detected than this number then these
  // contacts will be ignored and bodies will start interpenetrating / fall
  // through the world. Note: This value is low because this is a simple test.
  // For a real project use something in the order of 65536.
  constexpr auto kMaxContactConstraints = 65536;

  // Create mapping table from object layer to broadphase layer
  // Note: As this is an interface, PhysicsSystem will take a reference to this
  // so this instance needs to stay alive!
  m_broadPhaseLayerInterface = std::make_unique<BPLayerInterface>();
  m_objectVsBroadPhaseLayerFilter =
    std::make_unique<JPH::ObjectVsBroadPhaseLayerFilter>();
  m_objectVsObjectLayerFilter = std::make_unique<ObjectLayerPairFilter>();

  m_physicsSystem.Init(kMaxBodies, kNumBodyMutexes, kMaxBodyPairs,
                       kMaxContactConstraints, *m_broadPhaseLayerInterface,
                       *m_objectVsBroadPhaseLayerFilter,
                       *m_objectVsObjectLayerFilter);

  // We need a temp allocator for temporary allocations during the physics
  // update.
#if 0
  // We're pre-allocating 10 MB to avoid having to do allocations during
  // the physics update. B.t.w. 10 MB is way too much for this example but it is
  // a typical value you can use. If you don't want to pre-allocate you can also
  // use TempAllocatorMalloc to fall back to malloc / free.
  m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
#else
  m_tempAllocator = std::make_unique<JPH::TempAllocatorMalloc>();
#endif

  // Maximum amount of jobs to allow
  constexpr auto kMaxPhysicsJobs = 2048;

  // Maximum amount of barriers to allow
  constexpr auto kMaxPhysicsBarriers = 8;

  // We need a job system that will execute physics jobs on multiple threads.
  // Typically you would implement the JobSystem interface yourself and let Jolt
  // Physics run on top of your own job scheduler. JobSystemThreadPool is an
  // example implementation.

  const auto numThreads = std::thread::hardware_concurrency() - 1;
  m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
    kMaxPhysicsJobs, kMaxPhysicsBarriers, numThreads);
}

void PhysicsWorld::enableDebugDraw(bool b) { m_debugDrawEnabled = b; }
bool PhysicsWorld::isDebugDrawEnabled() const { return m_debugDrawEnabled; }

void PhysicsWorld::setGravity(const glm::vec3 &v) {
  m_physicsSystem.SetGravity(to_Jolt(v));
}
glm::vec3 PhysicsWorld::getGravity() const {
  return to_glm(m_physicsSystem.GetGravity());
}

JPH::Body *PhysicsWorld::createBody(const Transform &xf,
                                    const RigidBodySettings &settings,
                                    const JPH::Shape *shape) {
  auto &bodyInterface = m_physicsSystem.GetBodyInterface();
  auto *body = bodyInterface.CreateBody(makeSettings(xf, settings, shape));
  bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);
  return body;
}
void PhysicsWorld::destroyBody(JPH::BodyID bodyId) {
  auto &bodyInterface = m_physicsSystem.GetBodyInterface();
  bodyInterface.RemoveBody(bodyId);
  bodyInterface.DestroyBody(bodyId);
}

JPH::Ref<JPH::Character>
PhysicsWorld::createCharacter(const Transform &xf,
                              const CharacterSettings &settings,
                              const JPH::Shape *shape) {
  const auto characterSettings = makeSettings(settings, shape);
  constexpr auto kNoUserData = 0;

  JPH::Ref<JPH::Character> character = new JPH::Character{
    &characterSettings, to_Jolt(xf.getPosition()), to_Jolt(xf.getOrientation()),
    kNoUserData, &m_physicsSystem};
  character->AddToPhysicsSystem(JPH::EActivation::Activate);
  return character;
}

void PhysicsWorld::simulate(float deltaTime) {
  // If you take larger steps than 1 / 60th of a second you need to do multiple
  // collision steps in order to keep the simulation stable. Do 1 collision step
  // per 1 / 60th of a second (round up).
  constexpr auto kCollisionSteps = 1;
  m_physicsSystem.Update(deltaTime, kCollisionSteps, m_tempAllocator.get(),
                         m_jobSystem.get());
}
void PhysicsWorld::debugDraw(DebugDraw &dd) {
  if (!m_debugDrawEnabled) return;

  JoltPhysics::debugRenderer->setSource(dd);
  const JPH::BodyManager::DrawSettings settings{
    .mDrawShape = false, // ... not supported (yet?).
    .mDrawBoundingBox = true,
    .mDrawWorldTransform = true,
  };
  m_physicsSystem.DrawBodies(settings, JoltPhysics::debugRenderer, nullptr);
}
