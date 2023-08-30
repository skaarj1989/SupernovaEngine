#include "LuaNameComponent.hpp"
#include "sol/state.hpp"

#include "NameComponent.hpp"
#include "Sol2HelperMacros.hpp"

#include <format>

void registerNameComponent(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(NameComponent,
    sol::call_constructor,
    sol::constructors<NameComponent(const std::string &)>(),

    _BIND(NameComponent, name),

    BIND_TYPEID(NameComponent),
    sol::meta_function::to_string, [](const NameComponent &self) {
      return std::format("NameComponent({})", self.name);
    }
  );
  // clang-format on
}
