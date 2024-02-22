#include "LuaInputSystem.hpp"
#include "sol/state.hpp"

#include "os/InputSystem.hpp"
#include "Sol2HelperMacros.hpp"

void registerInputSystem(sol::state &lua) {
  using os::InputSystem;

#define BIND(Member) _BIND(InputSystem, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(InputSystem,
    sol::no_constructor,

    BIND(setMousePosition),

    BIND(getMousePosition),
    BIND(getMouseDelta),
    
    BIND(showCursor),
    BIND(isCursorVisible),

    BIND(isMouseDown),
    BIND(isMouseUp),

    BIND(isKeyDown),
    BIND(isKeyUp),

    BIND_TOSTRING(InputSystem)
  );
  // clang-format on
#undef BIND
}
