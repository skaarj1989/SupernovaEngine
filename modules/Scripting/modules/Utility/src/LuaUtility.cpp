#include "LuaUtility.hpp"
#include "sol/state.hpp"
#include "entt/core/hashed_string.hpp"
#include "ScriptTypeInfo.hpp"

void registerUtility(sol::state &lua) {
  lua["hashString"] = [](const char *str) {
    return entt::hashed_string{str}.value();
  };

  lua["typeOf"] = [](const sol::table &obj) { return getTypeId(obj); };

  lua["isTypeOf"] = [](const sol::table &obj, const sol::table &type) {
    const auto testedType = getTypeId(type);
    return testedType && getTypeId(obj) == testedType;
  };
}
