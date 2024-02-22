#include "LuaPhysics.hpp"
#include "sol/state.hpp"

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"
#include "physics/ShapeBuilder.hpp"

#include "LuaEmitter.hpp"
#include "Sol2HelperMacros.hpp"

namespace {

void registerPhysicsWorld(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(RayCastBPResult, Member)
  lua.DEFINE_USERTYPE(RayCastBPResult,
    sol::no_constructor,

    BIND(position),
    BIND(entityId),

    BIND_TOSTRING(RayCastBPResult)
  );
#undef BIND

#define BIND(Member) _BIND(RayCastNPResult, Member)
  lua.DEFINE_USERTYPE(RayCastNPResult,
    sol::no_constructor,

    BIND(position),
    BIND(normal),
    BIND(entityId),

    BIND_TOSTRING(RayCastNPResult)
  );
#undef BIND

#define BIND(Member) _BIND(PhysicsWorld, Member)
	lua.DEFINE_USERTYPE(PhysicsWorld,
    sol::no_constructor,

    BIND(setDebugDrawFlags),
    BIND(getDebugDrawFlags),

    BIND(setGravity),
    BIND(getGravity),
    
    BIND(castRayBP),
    BIND(castRayNP),

    BIND_TOSTRING(PhysicsWorld)
  );
#undef BIND

#define MAKE_PAIR(Value) _MAKE_PAIR(PhysicsWorld::DebugDrawFlags, Value)
  lua DEFINE_NESTED_ENUM(PhysicsWorld, DebugDrawFlags, {
    MAKE_PAIR(None),
    MAKE_PAIR(Shape),
    MAKE_PAIR(BoundingBox),
    MAKE_PAIR(WorldTransform),
  });
#undef MAKE_PAIR
  // clang-format on
}

template <typename Component> void registerMetaEvents() {
  LuaEmitter::createMetaEvent<Component, CollisionStartedEvent>();
  LuaEmitter::createMetaEvent<Component, CollisionEndedEvent>();
}

void registerEvents(sol::state &lua) {
  registerMetaEvents<RigidBody>();
  registerMetaEvents<Character>();
  registerMetaEvents<CharacterVirtual>();

#define BIND(Member) _BIND(CollisionStartedEvent, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(CollisionStartedEvent,
    sol::no_constructor,

    BIND(other),
    BIND(offset),
    BIND(normal),

    BIND_TYPEID(CollisionStartedEvent),
    BIND_TOSTRING(CollisionStartedEvent)
  );
#undef BIND

#define BIND(Member) _BIND(CollisionEndedEvent, Member)
  lua.DEFINE_USERTYPE(CollisionEndedEvent,
    sol::no_constructor,
    
    BIND(other),

    BIND_TYPEID(CollisionEndedEvent),
    BIND_TOSTRING(CollisionEndedEvent)
  );
#undef BIND
  // clang-format on
}

void registerShapeBuilder(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(UncookedShape,
    sol::no_constructor,

    "isValid", &UncookedShape::IsValid,

    sol::meta_function::to_string, [](const UncookedShape &self) {
      return std::format("UncookedShape({})", self.IsValid() ? "Valid" : "Invalid");
    }
  );

#define BIND(Member) _BIND(ShapeBuilder, Member)
  lua.DEFINE_USERTYPE(ShapeBuilder,
    sol::call_constructor,
    sol::constructors<ShapeBuilder()>(),

    BIND(makeSphere),
    BIND(makeBox),
    BIND(makeCapsule),
    BIND(makeConvexHull),

    BIND(set),
    BIND(reset),

    BIND(add),
    BIND(scale),
    BIND(rotateTranslate),

    BIND(size),

    BIND(build),

    BIND_TOSTRING(ShapeBuilder)
  );
#undef BIND
  // clang-format on
}

void registerResources(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(ColliderResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource>(),
    
    BIND_TOSTRING(ColliderResource)
  );
  // clang-format on
  lua["loadCollider"] = loadResource<ColliderManager>;
}

void registerColliderComponent(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(ColliderComponent,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<ColliderResource> resource) {
        return ColliderComponent{resource};
      },
      [](const UncookedShape &uncooked) {
        return ColliderComponent{
          uncooked.IsValid()
            ? std::make_shared<ColliderResource>(uncooked.Get(), "")
            : nullptr};
      }),

    _BIND(ColliderComponent, resource),

    BIND_TYPEID(ColliderComponent),
    sol::meta_function::to_string, [](const ColliderComponent &self) {
      return std::format("ColliderComponent({})", self.resource ? "Valid" : "Invalid");
    }
  );
  // clang-format on
}

void registerCollisionLayer(sol::state &lua) {
#define BIND(Member) _BIND(CollisionLayer, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(CollisionLayer,
    sol::call_constructor,
    sol::constructors<
      CollisionLayer(),
      CollisionLayer(uint8_t, uint8_t)
    >(),

    BIND(group),
    BIND(mask),

    sol::meta_function::to_string,
      [](const CollisionLayer &self) {
        return std::format("CollisionLayer({})", self.group);
      }
  );
  // clang-format on
#undef BIND
}

void registerRigidBodyComponent(sol::state &lua) {
  // -- MotionType enum:

  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(MotionType, Key)
  lua.DEFINE_ENUM(MotionType, {
    MAKE_PAIR(Dynamic),
    MAKE_PAIR(Static),
    MAKE_PAIR(Kinematic),
  });
#undef MAKE_PAIR

  // -- RigidBody class:

#define BIND(Member) _BIND(RigidBody, Member)
  lua.DEFINE_USERTYPE(RigidBody,
    sol::call_constructor,
    sol::constructors<
      RigidBody(),
      RigidBody(const RigidBody::Settings &)
    >(),

    "connect", LuaEmitter::makeConnectLambda<RigidBody>(),
    "disconnect", LuaEmitter::makeDisconnectLambda<RigidBody>(),
    
    BIND(getSettings),

    BIND(setPosition),
    BIND(setRotation),
    BIND(setLinearVelocity),

    BIND(applyImpulse),
    
    BIND(getPosition),
    BIND(getRotation),
    BIND(getLinearVelocity),

    BIND_TYPEID(RigidBody),
    BIND_TOSTRING(RigidBody)
  );
#undef BIND

  // -- RigidBody::Settings struct:

#define BIND(Member) _BIND(RigidBody::Settings, Member)
  lua DEFINE_NESTED_USERTYPE(RigidBody, Settings, 
    sol::call_constructor,
    sol::factories(
      [] { return RigidBody::Settings{}; },
      [](const sol::table &t) {
        return RigidBody::Settings{
          CAPTURE_FIELD_T(layer, CollisionLayer, {}),
          CAPTURE_FIELD(isSensor, false),
          CAPTURE_FIELD(mass, 1.0f),
          CAPTURE_FIELD(motionType, MotionType::Static),
          CAPTURE_FIELD(friction, 0.2f),
          CAPTURE_FIELD(restitution, 0.0f),
          CAPTURE_FIELD(linearDamping, 0.05f),
          CAPTURE_FIELD(angularDamping, 0.05f),
          CAPTURE_FIELD(gravityFactor, 1.0f),
        };
      }
    ),

    BIND(layer),
    BIND(mass),
    BIND(motionType),
    BIND(friction),
    BIND(restitution),
    BIND(linearDamping),
    BIND(angularDamping),
    BIND(gravityFactor),

    BIND_TOSTRING(RigidBody::Settings)
  );
#undef BIND
  // clang-format on
}
void registerCharacterComponent(sol::state &lua) {
  // -- EGroundState enum:

  using GroundState = JPH::Character::EGroundState;

  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(GroundState, Key)
  lua.DEFINE_ENUM(GroundState, {
    MAKE_PAIR(OnGround),
    MAKE_PAIR(OnSteepGround),
    MAKE_PAIR(NotSupported),
    MAKE_PAIR(InAir),
  });
#undef MAKE_PAIR

  // -- Character class:

#define BIND(Member) _BIND(Character, Member)
  lua.DEFINE_USERTYPE(Character,
    sol::call_constructor,
    sol::constructors<
      Character(),
      Character(const Character::Settings &)
    >(),

    "connect", LuaEmitter::makeConnectLambda<Character>(),
    "disconnect", LuaEmitter::makeDisconnectLambda<Character>(),

    BIND(getSettings),

    BIND(setRotation),
    BIND(setLinearVelocity),
    
    BIND(getPosition),
    BIND(getRotation),
    BIND(getLinearVelocity),

    BIND(getGroundState),
    BIND(isSupported),
    BIND(getGroundNormal),
    BIND(getGroundVelocity),

    BIND_TYPEID(Character),
    BIND_TOSTRING(Character)
  );
#undef BIND

  // -- Character::Settings struct:

#define BIND(Member) _BIND(Character::Settings, Member)
  lua DEFINE_NESTED_USERTYPE(Character, Settings, 
    sol::call_constructor,
    sol::factories(
      [] { return Character::Settings{}; },
      [](const sol::table &t) {
        return Character::Settings{
          CAPTURE_FIELD(maxSlopeAngle, 50.0f),
          CAPTURE_FIELD_T(layer, CollisionLayer, {}),

          CAPTURE_FIELD(mass, 80.0f),
          CAPTURE_FIELD(friction, 0.2f),
          CAPTURE_FIELD(gravityFactor, 1.0f),
        };
      }
    ),

    BIND(maxSlopeAngle),
    BIND(layer),
    BIND(mass),
    BIND(friction),
    BIND(gravityFactor),

    BIND_TOSTRING(Character::Settings)
  );
#undef BIND
  // clang-format on
}

void registerCharacterVirtualComponent(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(CharacterVirtual, Member)
  lua.DEFINE_USERTYPE(CharacterVirtual,
    sol::call_constructor,
    sol::constructors<
      CharacterVirtual(),
      CharacterVirtual(const CharacterVirtual::Settings &)
    >(),

    "connect", LuaEmitter::makeConnectLambda<CharacterVirtual>(),
    "disconnect", LuaEmitter::makeDisconnectLambda<CharacterVirtual>(),

    BIND(getSettings),
    
    BIND(setRotation),
    BIND(setLinearVelocity),
    
    "stickToFloor", sol::property(
      &CharacterVirtual::isStickToFloor,
      &CharacterVirtual::setStickToFloor
    ),
    "walkStairs", sol::property(
      &CharacterVirtual::canWalkStairs,
      &CharacterVirtual::setWalkStairs
    ),

    BIND(getUp),

    BIND(getPosition),
    BIND(getRotation),
    BIND(getLinearVelocity),

    BIND(getGroundState),
    BIND(isSupported),
    BIND(getGroundNormal),
    BIND(getGroundVelocity),

    BIND_TYPEID(CharacterVirtual),
    BIND_TOSTRING(CharacterVirtual)
  );
#undef BIND

  // -- CharacterVirtual::Settings struct:

#define BIND(Member) _BIND(CharacterVirtual::Settings, Member)
  lua DEFINE_NESTED_USERTYPE(CharacterVirtual, Settings,
    sol::call_constructor,
    sol::factories(
      [] { return CharacterVirtual::Settings{}; },
      [](const sol::table &t) {
        return CharacterVirtual::Settings{
          CAPTURE_FIELD(maxSlopeAngle, 50.0f),
          CAPTURE_FIELD_T(layer, CollisionLayer, {}),
          CAPTURE_FIELD(mass, 70.0f),

          CAPTURE_FIELD(maxStrength, 100.0f),
          CAPTURE_FIELD_T(shapeOffset, glm::vec3, 0.0f),
          CAPTURE_FIELD(predictiveContactDistance, 0.1f),
          CAPTURE_FIELD(maxCollisionIterations, 5u),
          CAPTURE_FIELD(maxConstraintIterations, 15u),
          CAPTURE_FIELD(minTimeRemaining, 1.0e-4f),
          CAPTURE_FIELD(collisionTolerance, 1.0e-3f),
          CAPTURE_FIELD(characterPadding, 0.02f),
          CAPTURE_FIELD(maxNumHits, 256u),
          CAPTURE_FIELD(hitReductionCosMaxAngle, 0.999f),
          CAPTURE_FIELD(penetrationRecoverySpeed, 1.0f),
        };
      }
    ),

    BIND(maxSlopeAngle),
    BIND(layer),
    BIND(mass),

    BIND(maxStrength),
    BIND(shapeOffset),
    BIND(predictiveContactDistance),
    BIND(maxCollisionIterations),
    BIND(maxConstraintIterations),
    BIND(minTimeRemaining),
    BIND(collisionTolerance),
    BIND(characterPadding),
    BIND(maxNumHits),
    BIND(hitReductionCosMaxAngle),
    BIND(penetrationRecoverySpeed),

    BIND_TOSTRING(CharacterVirtual::Settings)
  );
#undef BIND
  // clang-format on
}

} // namespace

void registerJoltPhysics(sol::state &lua) {
  registerPhysicsWorld(lua);

  registerEvents(lua);
  registerShapeBuilder(lua);
  registerResources(lua);

  registerColliderComponent(lua);
  registerCollisionLayer(lua);
  registerRigidBodyComponent(lua);
  registerCharacterComponent(lua);
  registerCharacterVirtualComponent(lua);
}
