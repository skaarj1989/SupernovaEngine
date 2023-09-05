#include "physics/PhysicsWorld.hpp"

#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"

#include "physics/JoltPhysics.hpp"
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

[[nodiscard]] auto makeSettings(const RigidBody::Settings &settings,
                                const PhysicsWorld::CreateInfo &createInfo) {
  assert(createInfo.shape);

  JPH::BodyCreationSettings out{
    createInfo.shape,
    to_Jolt(createInfo.transform.getPosition()),
    to_Jolt(createInfo.transform.getOrientation()),
    convert(settings.motionType),
    encode(settings.layer),
  };
  out.mFriction = settings.friction;
  out.mRestitution = settings.restitution;
  out.mLinearDamping = glm::clamp(settings.linearDamping, 0.0f, 1.0f);
  out.mAngularDamping = glm::clamp(settings.angularDamping, 0.0f, 1.0f);
  out.mGravityFactor = settings.gravityFactor;
  out.mUserData = createInfo.userData;

  using enum JPH::EOverrideMassProperties;

  if (settings.motionType == MotionType::Static ||
      createInfo.shape->MustBeStatic()) {
    out.mAllowDynamicOrKinematic = false;
    out.mOverrideMassProperties = MassAndInertiaProvided;
  } else {
    out.mAllowDynamicOrKinematic = true;
    if (settings.mass > 0) {
      out.mOverrideMassProperties = CalculateInertia;
      out.mMassPropertiesOverride.mMass = settings.mass;
    } else {
      out.mOverrideMassProperties = CalculateMassAndInertia;
    }
  }
  return out;
}
[[nodiscard]] auto makeSettings(const Character::Settings &settings,
                                const JPH::Shape *shape) {
  assert(shape);

  JPH::CharacterSettings out;
  out.mMaxSlopeAngle = glm::radians(settings.maxSlopeAngle);
  out.mShape = shape;
  out.mLayer = encode(settings.layer);

  out.mMass = settings.mass;
  out.mFriction = settings.friction;
  out.mGravityFactor = settings.gravityFactor;
  return out;
}
[[nodiscard]] auto makeSettings(const CharacterVirtual::Settings &settings,
                                const JPH::Shape *shape) {
  assert(createInfo.shape);

  JPH::CharacterVirtualSettings out;
  out.mMaxSlopeAngle = glm::radians(settings.maxSlopeAngle);
  out.mShape = shape;

  out.mMass = settings.mass;
  out.mMaxStrength = settings.maxStrength;
  out.mBackFaceMode = settings.backFaceMode;
  out.mPredictiveContactDistance = settings.predictiveContactDistance;
  out.mMaxCollisionIterations = settings.maxCollisionIterations;
  out.mMaxConstraintIterations = settings.maxConstraintIterations;
  out.mMinTimeRemaining = settings.minTimeRemaining;
  out.mCollisionTolerance = settings.collisionTolerance;
  out.mCharacterPadding = settings.characterPadding;
  out.mMaxNumHits = settings.maxNumHits;
  out.mHitReductionCosMaxAngle = settings.hitReductionCosMaxAngle;
  out.mPenetrationRecoverySpeed = settings.penetrationRecoverySpeed;
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

void PhysicsWorld::initBody(RigidBody &rb, const CreateInfo &createInfo) {
  rb.m_bodyId = _createBody(rb.m_settings, createInfo);
  rb.m_joltPhysics = &m_physicsSystem;
}
void PhysicsWorld::initCharacter(Character &c, const CreateInfo &createInfo) {
  c.m_character = _createCharacter(c.m_settings, createInfo);
}
void PhysicsWorld::initCharacter(CharacterVirtual &c,
                                 const CreateInfo &createInfo) {
  c.m_character = _createCharacter(c.m_settings, createInfo);
}

void PhysicsWorld::remove(const RigidBody &rb) { _destroy(rb.getBodyId()); }
void PhysicsWorld::remove(const Character &c) {
  if (auto &character = c.m_character; character) {
    character->RemoveFromPhysicsSystem();
  }
}

void PhysicsWorld::setCollisionShape(RigidBody &rb, const JPH::Shape *shape) {
  if (rb && shape) {
    getBodyInterface().SetShape(rb.getBodyId(), shape, false,
                                JPH::EActivation::DontActivate);
  }
}
void PhysicsWorld::setCollisionShape(Character &c, const JPH::Shape *shape) {
  if (c && shape) c.m_character->SetShape(shape, FLT_MAX);
}
void PhysicsWorld::setCollisionShape(CharacterVirtual &c,
                                     const JPH::Shape *shape) {
  if (c && shape) {
    const auto layer = encode(c.m_settings.layer);
    // clang-format off
    c.m_character->SetShape(
      shape,
      1.5f * m_physicsSystem.GetPhysicsSettings().mPenetrationSlop,
      m_physicsSystem.GetDefaultBroadPhaseLayerFilter(layer),
      m_physicsSystem.GetDefaultLayerFilter(layer),
      {},
      {},
      *m_tempAllocator
    );
    // clang-format on
  }
}

JPH::BodyManager::BodyStats PhysicsWorld::getBodyStats() const {
  return m_physicsSystem.GetBodyStats();
}

void PhysicsWorld::update(const Character &c, Transform *xf,
                          float collisionTollerance) {
  if (const auto &character = c.m_character; character) {
    character->PostSimulation(collisionTollerance);
    if (xf) setTransform(character->GetWorldTransform(), *xf);
  }
}
void PhysicsWorld::update(const CharacterVirtual &c, Transform *xf,
                          float timeStep) {
  const auto &character = c.m_character;
  if (!character) return;

  JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
  if (c.isStickToFloor()) {
    updateSettings.mStickToFloorStepDown =
      -character->GetUp() * updateSettings.mStickToFloorStepDown.Length();
  } else {
    updateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();
  }
  if (c.canWalkStairs()) {
    updateSettings.mWalkStairsStepUp =
      character->GetUp() * updateSettings.mWalkStairsStepUp.Length();
  } else {
    updateSettings.mWalkStairsStepUp = JPH::Vec3::sZero();
  }

  const auto layer = encode(c.m_settings.layer);

  // clang-format off
  character->ExtendedUpdate(
    timeStep,
    -character->GetUp() * m_physicsSystem.GetGravity().Length(),
    updateSettings,
    m_physicsSystem.GetDefaultBroadPhaseLayerFilter(layer),
    m_physicsSystem.GetDefaultLayerFilter(layer),
    {},
    {},
    *m_tempAllocator
  );
  // clang-format on

  if (xf) setTransform(character->GetWorldTransform(), *xf);
}

void PhysicsWorld::simulate(float timeStep) {
  // If you take larger steps than 1 / 60th of a second you need to do multiple
  // collision steps in order to keep the simulation stable. Do 1 collision step
  // per 1 / 60th of a second (round up).
  constexpr auto kCollisionSteps = 1;
  m_physicsSystem.Update(timeStep, kCollisionSteps, m_tempAllocator.get(),
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

//
// (private):
//

void PhysicsWorld::_destroy(const JPH::BodyID bodyId) {
  if (!bodyId.IsInvalid()) {
    auto &bodyInterface = m_physicsSystem.GetBodyInterface();
    bodyInterface.RemoveBody(bodyId);
    bodyInterface.DestroyBody(bodyId);
  }
}

JPH::BodyID PhysicsWorld::_createBody(const RigidBody::Settings &settings,
                                      const CreateInfo &createInfo) {
  return m_physicsSystem.GetBodyInterface().CreateAndAddBody(
    makeSettings(settings, createInfo), JPH::EActivation::Activate);
}
JPH::Ref<JPH::Character>
PhysicsWorld::_createCharacter(const Character::Settings &settings,
                               const CreateInfo &createInfo) {
  const auto characterSettings = makeSettings(settings, createInfo.shape);

  JPH::Ref<JPH::Character> character = new JPH::Character{
    &characterSettings,
    to_Jolt(createInfo.transform.getPosition()),
    to_Jolt(createInfo.transform.getOrientation()),
    createInfo.userData,
    &m_physicsSystem,
  };
  character->AddToPhysicsSystem(JPH::EActivation::Activate);
  return character;
}
JPH::Ref<JPH::CharacterVirtual>
PhysicsWorld::_createCharacter(const CharacterVirtual::Settings &settings,
                               const CreateInfo &createInfo) {
  const auto characterSettings = makeSettings(settings, createInfo.shape);

  JPH::Ref<JPH::CharacterVirtual> character = new JPH::CharacterVirtual{
    &characterSettings,
    to_Jolt(createInfo.transform.getPosition()),
    to_Jolt(createInfo.transform.getOrientation()),
    &m_physicsSystem,
  };
  return character;
}

//
// Helper:
//

void setTransform(const JPH::RMat44 &src, Transform &dst) {
  dst.setPosition(to_glm(src.GetTranslation()))
    .setOrientation(to_glm(src.GetRotation()));
}
