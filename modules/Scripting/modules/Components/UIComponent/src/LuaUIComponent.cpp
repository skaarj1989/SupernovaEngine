#include "LuaUIComponent.hpp"
#include "sol/state.hpp"

#include "UIComponent.hpp"
#include "Sol2HelperMacros.hpp"
#include <format>

void registerUIComponent(sol::state &lua) {
  // clang-format off
  DEFINE_USERTYPE(UIComponent,
    sol::call_constructor,
    sol::constructors<UIComponent()>(),

    _BIND(UIComponent, context),

    BIND_TYPEID(UIComponent),
    sol::meta_function::to_string, [](const UIComponent &self) {
      return std::format("UIComponent({})",
                         self.context ? self.context->GetName() : "undefined");
    }
  );
  // clang-format on
}
