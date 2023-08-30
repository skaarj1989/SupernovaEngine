#pragma once

#include "sol/state.hpp"

struct ScriptContext {
  explicit ScriptContext(sol::state &);

  sol::state *lua{nullptr};
  sol::environment defaultEnv;
};
