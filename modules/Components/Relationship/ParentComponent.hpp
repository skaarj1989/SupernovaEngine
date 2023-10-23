#pragma once

#include "entt/core/type_info.hpp"
#include "entt/entity/entity.hpp"

struct ParentComponent {
  entt::entity parent{entt::null};
};

template <> struct entt::type_hash<ParentComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3912849999;
  }
};
