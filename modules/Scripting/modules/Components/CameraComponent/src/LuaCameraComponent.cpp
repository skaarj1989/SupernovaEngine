#include "LuaCameraComponent.hpp"
#include "sol/state.hpp"

#include "CameraComponent.hpp"
#include "Sol2HelperMacros.hpp"

void registerCameraComponent(sol::state &lua) {
#define BIND(Member) _BIND(CameraComponent, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(CameraComponent,
    sol::call_constructor,
    sol::constructors<
      CameraComponent(),
      CameraComponent(rhi::Extent2D)
    >(),

    BIND(extent),
    "target", sol::readonly_property(
      [](const CameraComponent &self) { return self.target; }
    ),
    BIND(camera),
    BIND(renderSettings),
    BIND(skyLight),
    BIND(postProcessEffects),
    BIND(debugDraw),

    BIND_TYPEID(CameraComponent),
    BIND_TOSTRING(CameraComponent)
  );
  // clang-format on
#undef BIND
}
