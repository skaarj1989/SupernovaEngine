#include "LuaLogger.hpp"
#include "sol/state.hpp"
#include "spdlog/spdlog.h"

void registerLogger(sol::state &lua) {
  auto m = lua["spdlog"].get_or_create<sol::table>();

  m["log"] = [](const std::string_view msg) { spdlog::info(msg); };
}
