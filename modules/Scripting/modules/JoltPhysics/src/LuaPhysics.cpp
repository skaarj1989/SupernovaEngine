#include "LuaPhysics.hpp"
#include "sol/state.hpp"

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"
#include "physics/ColliderManager.hpp"
#include "physics/RigidBody.hpp"
#include "physics/Character.hpp"

#include "ScriptTypeInfo.hpp"
#include "MetaHelper.hpp"
#include "LuaEmitter.hpp"

#include "Sol2HelperMacros.hpp"

using namespace entt::literals;

namespace {

void registerPhysicsWorld(sol::state &lua) {
#define BIND(Member) _BIND(PhysicsWorld, Member)
  // clang-format off
	DEFINE_USERTYPE(PhysicsWorld,
    sol::no_constructor,

    BIND(setGravity),
    BIND(getGravity),

    BIND_TOSTRING(PhysicsWorld)
  );
  // clang-format on
#undef BIND
}

void registerEvents(sol::state &lua) {
  // TODO ...
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

#define CAPTURE_FIELD(name, defaultValue)                                      \
  .##name = t.get_or(#name, defaultValue)

#define CAPTURE_FIELD_T(name, T, defaultValue)                                 \
  .##name = t.get_or<const T &>(#name, T{defaultValue})

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

#define BIND(Member) _BIND(RigidBodySettings, Member)

  // clang-format off
  DEFINE_USERTYPE(RigidBodySettings, 
    sol::call_constructor,
    sol::factories(
      [] { return RigidBodySettings{}; },
      [](const sol::table &t) {
        return RigidBodySettings{
          CAPTURE_FIELD_T(layer, CollisionLayer, {}),
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
    
    // Do not expose 'setCollisionShape' (it's reserved for PhysicsSystem).
   
    BIND(getSettings),

    BIND(setLinearVelocity),
    BIND(getLinearVelocity),

    BIND(applyImpulse),

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

    // Do not expose 'setCollisionShape' (it is reserved for PhysicsSystem).

    BIND(getSettings),

    BIND(getRotation),
    BIND(setRotation),

    BIND(setLinearVelocity),
    BIND(getLinearVelocity),

    BIND(getGroundState),
    BIND(getGroundNormal),

    BIND_TYPEID(Character),
    BIND_TOSTRING(Character)
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
}
