#include "LuaPhysics.hpp"
#include "sol/state.hpp"

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"

#include "LuaEmitter.hpp"
#include "Sol2HelperMacros.hpp"

namespace {

void registerPhysicsWorld(sol::state &lua) {
  // clang-format off
  #define BIND(Member) _BIND(RayCastBPResult, Member)
  DEFINE_USERTYPE(RayCastBPResult,
    sol::no_constructor,

    BIND(position),
    BIND(entityId),

    BIND_TOSTRING(RayCastBPResult)
  );
#undef BIND

#define BIND(Member) _BIND(RayCastNPResult, Member)
  DEFINE_USERTYPE(RayCastNPResult,
    sol::no_constructor,

    BIND(position),
    BIND(normal),
    BIND(entityId),

    BIND_TOSTRING(RayCastNPResult)
  );
#undef BIND

#define BIND(Member) _BIND(PhysicsWorld, Member)
	DEFINE_USERTYPE(PhysicsWorld,
    sol::no_constructor,

    BIND(setGravity),
    BIND(getGravity),
    
    BIND(castRayBP),
    BIND(castRayNP),

    BIND_TOSTRING(PhysicsWorld)
  );
#undef BIND
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
  DEFINE_USERTYPE(CollisionStartedEvent,
    sol::no_constructor,

    BIND(other),
    BIND(offset),
    BIND(normal),

    BIND_TYPEID(CollisionStartedEvent),
    BIND_TOSTRING(CollisionStartedEvent)
  );
#undef BIND

#define BIND(Member) _BIND(CollisionEndedEvent, Member)
  DEFINE_USERTYPE(CollisionEndedEvent,
    sol::no_constructor,
    
    BIND(other),

    BIND_TYPEID(CollisionEndedEvent),
    BIND_TOSTRING(CollisionEndedEvent)
  );
#undef BIND
  // clang-format on
}

void registerResources(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(ColliderResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource>(),
    
    BIND_TOSTRING(ColliderResource)
  );
  lua["loadCollider"] = loadResource<ColliderManager>;
  // clang-format on
}

void registerColliderComponent(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(ColliderComponent,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<ColliderResource> resource) {
        return ColliderComponent{resource};
      }
    ),

    _BIND(ColliderComponent, resource),

    BIND_TYPEID(ColliderComponent),
    BIND_TOSTRING(ColliderComponent)
  );
  // clang-format on
}

void registerCollisionLayer(sol::state &lua) {
#define BIND(Member) _BIND(CollisionLayer, Member)

  // clang-format off
  DEFINE_USERTYPE(CollisionLayer,
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

#define CAPTURE_FIELD(name, defaultValue) .name = t.get_or(#name, defaultValue)
#define CAPTURE_FIELD_T(name, T, defaultValue)                                 \
  .name = t.get_or<const T &>(#name, T{defaultValue})

void registerRigidBodyComponent(sol::state &lua) {
  // -- MotionType enum:

#define MAKE_PAIR(Key) _MAKE_PAIR(MotionType, Key)
  DEFINE_ENUM(MotionType, {
                            MAKE_PAIR(Dynamic),
                            MAKE_PAIR(Static),
                            MAKE_PAIR(Kinematic),
                          });
#undef MAKE_PAIR

  // -- RigidBodySettings struct:

  using RigidBodySettings = RigidBody::Settings;

#define BIND(Member) _BIND(RigidBodySettings, Member)

  // clang-format off
  DEFINE_USERTYPE(RigidBodySettings, 
    sol::call_constructor,
    sol::factories(
      [] { return RigidBodySettings{}; },
      [](const sol::table &t) {
        return RigidBodySettings{
          CAPTURE_FIELD_T(layer, CollisionLayer, {}),
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

    BIND_TOSTRING(RigidBodySettings)
  );
#undef BIND

  // -- RigidBody class:

#define BIND(Member) _BIND(RigidBody, Member)
  DEFINE_USERTYPE(RigidBody,
    sol::call_constructor,
    sol::constructors<
      RigidBody(),
      RigidBody(const RigidBodySettings &)
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
  // clang-format on
#undef BIND
}
void registerCharacterComponent(sol::state &lua) {
  // -- EGroundState enum:

#define MAKE_PAIR(Key) _MAKE_PAIR(JPH::Character::EGroundState, Key)
  lua.new_enum<JPH::Character::EGroundState>("GroundState",
                                             {
                                               MAKE_PAIR(OnGround),
                                               MAKE_PAIR(OnSteepGround),
                                               MAKE_PAIR(NotSupported),
                                               MAKE_PAIR(InAir),
                                             });
#undef MAKE_PAIR

  // -- CharacterSettings struct:

  using CharacterSettings = Character::Settings;

#define BIND(Member) _BIND(CharacterSettings, Member)

  // clang-format off
  DEFINE_USERTYPE(CharacterSettings, 
    sol::call_constructor,
    sol::factories(
      [] { return CharacterSettings{}; },
      [](const sol::table &t) {
        return CharacterSettings{
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

    BIND_TOSTRING(CharacterSettings)
  );
#undef BIND

  // -- Character class:

#define BIND(Member) _BIND(Character, Member)
  // clang-format off
  DEFINE_USERTYPE(Character,
    sol::call_constructor,
    sol::constructors<
      Character(),
      Character(const CharacterSettings &)
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
  // clang-format on
#undef BIND
}

void registerCharacterVirtualComponent(sol::state &lua) {
  using CharacterVirtualSettings = CharacterVirtual::Settings;

  // -- Settings struct:

#define BIND(Member) _BIND(CharacterVirtualSettings, Member)

  // clang-format off
  DEFINE_USERTYPE(CharacterVirtualSettings, 
    sol::call_constructor,
    sol::factories(
      [] { return CharacterVirtualSettings{}; },
      [](const sol::table &t) {
        return CharacterVirtualSettings{
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

    BIND_TOSTRING(CharacterVirtualSettings)
  );
#undef BIND

#define BIND(Member) _BIND(CharacterVirtual, Member)
  // clang-format off
  DEFINE_USERTYPE(CharacterVirtual,
    sol::call_constructor,
    sol::constructors<
      CharacterVirtual(),
      CharacterVirtual(const CharacterVirtualSettings &)
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
  // clang-format on
#undef BIND
}

#undef CAPTURE_FIELD
#undef CAPTURE_FIELD_T

} // namespace

void registerJoltPhysics(sol::state &lua) {
  registerPhysicsWorld(lua);

  registerEvents(lua);
  registerResources(lua);

  registerColliderComponent(lua);
  registerCollisionLayer(lua);
  registerRigidBodyComponent(lua);
  registerCharacterComponent(lua);
  registerCharacterVirtualComponent(lua);
}
