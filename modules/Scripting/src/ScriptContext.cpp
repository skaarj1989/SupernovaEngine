#include "ScriptContext.hpp"

ScriptContext::ScriptContext(sol::state &s)
    : lua{std::addressof(s)}, defaultEnv{*lua, sol::create, lua->globals()} {}
