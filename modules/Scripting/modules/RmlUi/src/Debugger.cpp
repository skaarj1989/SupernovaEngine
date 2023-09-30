#include "Debugger.hpp"
#include "RmlUi/Debugger.h"

void registerDebugger(sol::table &lua) {
  auto m = lua["Debugger"].get_or_create<sol::table>();
  m.set_function("initialize", Rml::Debugger::Initialise);
  m.set_function("shutdown", Rml::Debugger::Shutdown);
  m.set_function("setContext", Rml::Debugger::SetContext);
  m.set_function("setVisible", Rml::Debugger::SetVisible);
  m.set_function("isVisible", Rml::Debugger::IsVisible);
}
