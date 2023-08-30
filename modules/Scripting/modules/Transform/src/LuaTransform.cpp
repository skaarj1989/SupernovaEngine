#include "LuaTransform.hpp"
#include "sol/state.hpp"

#include "Transform.hpp"
#include "Sol2HelperMacros.hpp"

void registerTransform(sol::state &lua) {
#define BIND(Member) _BIND(Transform, Member)
  // clang-format off
  DEFINE_USERTYPE(Transform,
    sol::call_constructor,
    sol::constructors<
      Transform(),
      Transform(const glm::vec3 &, const glm::quat &, const glm::vec3 &),
      Transform(const glm::vec3 &),
      Transform(const glm::mat4 &)
    >(),

    "Right", sol::readonly_property([]{ return Transform::kRight; }),
    "Up", sol::readonly_property([]{ return Transform::kUp; }),
    "Forward", sol::readonly_property([]{ return Transform::kForward; }),

    "load", [](Transform &self, const glm::mat4 &m) -> Transform & { 
      return self.load(m);
    },
    BIND(loadIdentity),

    BIND(setPosition),
    BIND(setOrientation),
    BIND(setEulerAngles),
    BIND(setScale),

    // ---

    BIND(getModelMatrix),
    BIND(getWorldMatrix),

    BIND(getLocalPosition),
    BIND(getLocalOrientation),
    BIND(getLocalEulerAngles),
    BIND(getLocalScale),
    
    BIND(getPosition),
    BIND(getOrientation),
    BIND(getEulerAngles),
    BIND(getScale),

    BIND(getRight),
    BIND(getUp),
    BIND(getForward),
    
    // ---

    BIND(translate),

    BIND(rotate),
    BIND(pitch),
    BIND(yaw),
    BIND(roll),
    
    "lookAt", sol::overload(
      sol::resolve<Transform &(const glm::vec4 &)>(&Transform::lookAt),
      sol::resolve<Transform &(const Transform &)>(&Transform::lookAt)
    ),

    BIND(scale),
    
    // ---

    BIND_TYPEID(Transform),
    BIND_TOSTRING(Transform)
  );
  // clang-format on
#undef BIND
}
