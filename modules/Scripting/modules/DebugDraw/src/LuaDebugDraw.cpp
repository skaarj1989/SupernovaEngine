#include "LuaDebugDraw.hpp"
#include "sol/state.hpp"

#include "DebugDraw.hpp"
#include "Sol2HelperMacros.hpp"

void registerDebugDraw(sol::state &lua) {
#define BIND(Member) _BIND(DebugDraw, Member)
  // clang-format off
  DEFINE_USERTYPE(DebugDraw,
    sol::no_constructor,

    "addPoint",
      sol::resolve<DebugDraw &(const glm::vec3 &, float, const glm::vec3 &)>(
        &DebugDraw::addPoint),
    "addLine",
      sol::resolve<DebugDraw &(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &)>(
       &DebugDraw::addLine),

    BIND(addCircle),
    BIND(addSphere),
    BIND(addAABB),
    BIND(addFrustum),

    BIND(empty),

    BIND_TOSTRING(DebugDraw)
  );
  // clang-format on
#undef BIND
}
