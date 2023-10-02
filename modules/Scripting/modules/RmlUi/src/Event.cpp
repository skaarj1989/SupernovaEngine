#include "Event.hpp"
#include "RmlUi/Core/Element.h"
#include "Rml2glm.hpp"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

void registerEvent(sol::table &lua) {
  // clang-format off
#define MAKE_PAIR(Value) _MAKE_PAIR(EventPhase, Value)
  DEFINE_ENUM(EventPhase, {
    MAKE_PAIR(None),
    MAKE_PAIR(Capture),
    MAKE_PAIR(Target),
    MAKE_PAIR(Bubble),
  });
#undef MAKE_PAIR

  DEFINE_USERTYPE(Event,
    sol::call_constructor,
    sol::constructors<
      Event(),
      Event(Element *, EventId, const String &, const Dictionary &, bool)
    >(),

    sol::meta_function::equal_to, sol::overload(
      sol::resolve<bool(const String &) const>(&Event::operator==),
      sol::resolve<bool(EventId) const>(&Event::operator==)
    ), 

    "getPhase", &Event::GetPhase,
    "setPhase", &Event::SetPhase,

    "setCurrentElement", &Event::SetCurrentElement,
    "getCurrentElement", &Event::GetCurrentElement,
    "getTargetElement", &Event::GetTargetElement,
    
    "getType", &Event::GetType,
    "getId", &Event::GetId,
    
    "stopPropagation", &Event::StopPropagation,
    "stopImmediatePropagation", &Event::StopImmediatePropagation,
    
    "isInterruptible", &Event::IsInterruptible,
    "isPropagating", &Event::IsPropagating,
    "isImmediatePropagating", &Event::IsImmediatePropagating,

    "getParameter", [](const Event &self, const String &name) {
      const auto &parameters = self.GetParameters();
      const auto it = parameters.find(name);
      return it != parameters.cend() ? it->second : Variant{};
    },

    "getUnprojectedMouseScreenPos", [](const Event &self) {
      return to_glm(self.GetUnprojectedMouseScreenPos());
    },

    BIND_TOSTRING(Event)
  );
  // clang-format on
}
