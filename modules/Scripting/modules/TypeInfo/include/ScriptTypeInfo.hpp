#pragma once

#include "entt/core/type_info.hpp"
#include "sol/table.hpp"
#include <optional>

[[nodiscard]] inline std::optional<entt::id_type>
getTypeId(const sol::table &obj) {
  if (auto f = obj["type_id"]; f.valid()) {
    return f().get<entt::id_type>();
  }
  return std::nullopt;
}
