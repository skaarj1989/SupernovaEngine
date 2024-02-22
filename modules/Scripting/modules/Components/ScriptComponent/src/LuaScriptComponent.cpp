#include "LuaScriptComponent.hpp"
#include "sol/state.hpp"

#include "ScriptComponent.hpp"
#include "Sol2HelperMacros.hpp"

namespace {

void registerScriptResource(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(ScriptResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource>(),

    BIND_TOSTRING(ScriptResource)
  );
  // clang-format on
  lua["loadScript"] = loadResource<ScriptManager>;
}

} // namespace

void registerScriptComponent(sol::state &lua) {
  registerScriptResource(lua);

  // clang-format off
  lua.DEFINE_USERTYPE(ScriptComponent,
    sol::call_constructor,
    sol::factories(
      [](std::shared_ptr<ScriptResource> resource) {
        return ScriptComponent{std::move(resource)};
      }
    ),

    _BIND(ScriptComponent, getResource),

    "self", sol::readonly_property(
      [](ScriptComponent &self) { return self.m_scriptNode.self(); }
    ),
    
    BIND_TYPEID(ScriptComponent),
    BIND_TOSTRING(ScriptComponent)
  );
  // clang-format on
}
