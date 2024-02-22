#include "LuaDebugDraw.hpp"
#include "sol/state.hpp"

#include "DebugDraw.hpp"
#include "Sol2HelperMacros.hpp"

void registerDebugDraw(sol::state &lua) {
#define BIND(Member) _BIND(DebugDraw, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(DebugDraw,
    sol::no_constructor,

    "addPoint", sol::overload(
      [](DebugDraw &self, const glm::vec3 &p, float size) -> DebugDraw & {
        return self.addPoint(p, size);
      },
      sol::resolve<DebugDraw &(const glm::vec3 &, float, const glm::vec3 &)>(&DebugDraw::addPoint)
    ),
    "addLine", sol::overload(
      [](DebugDraw &self, const glm::vec3 &start, const glm::vec3 &end) -> DebugDraw & {
        return self.addLine(start, end);
      },
      sol::resolve<DebugDraw &(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &)>(&DebugDraw::addLine)
    ),
    "addCircle", sol::overload(
      [](DebugDraw &self, float radius) -> DebugDraw & {
        return self.addCircle(radius);
      },
      [](DebugDraw &self, float radius, const glm::vec3 &color) -> DebugDraw & {
        return self.addCircle(radius, color);
      },
      &DebugDraw::addCircle
    ),
    "addSphere", sol::overload(
      [](DebugDraw &self, float radius) -> DebugDraw & {
        return self.addSphere(radius);
      },
      [](DebugDraw &self, float radius, const glm::vec3 &color) -> DebugDraw & {
        return self.addSphere(radius, color);
      },
      &DebugDraw::addSphere
    ),
    "addAABB", sol::overload(
      [](DebugDraw &self, const AABB &aabb) -> DebugDraw & {
        return self.addAABB(aabb);
      },
      &DebugDraw::addAABB
    ),
    "addFrustum", sol::overload(
      [](DebugDraw &self, const glm::mat4 &m) -> DebugDraw & {
        return self.addFrustum(m);
      },
      &DebugDraw::addFrustum
    ),

    BIND(empty),

    BIND_TOSTRING(DebugDraw)
  );
  // clang-format on
#undef BIND
}
