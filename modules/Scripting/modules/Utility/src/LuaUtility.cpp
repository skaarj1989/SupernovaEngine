#include "LuaUtility.hpp"
#include "sol/state.hpp"
#include "entt/core/hashed_string.hpp"
#include "ScriptTypeInfo.hpp"
#include <chrono>

void registerUtility(sol::state &lua) {
  lua["clock"] = [] {
    using namespace std::chrono;
    return static_cast<double>(
             duration_cast<microseconds>(
               high_resolution_clock::now().time_since_epoch())
               .count()) /
           std::micro::den;
  };

  lua["hashString"] = [](const char *str) {
    return entt::hashed_string{str}.value();
  };

  lua["typeOf"] = [](const sol::table &obj) { return getTypeId(obj); };

  lua["isTypeOf"] = [](const sol::table &obj, const sol::table &type) {
    const auto testedType = getTypeId(type);
    return testedType && getTypeId(obj) == testedType;
  };
}
