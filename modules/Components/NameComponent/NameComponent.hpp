#pragma once

#include "entt/core/type_info.hpp"
#include <string>

struct NameComponent {
  std::string name;

  template <class Archive> void serialize(Archive &archive) { archive(name); }
};

static_assert(std::is_copy_constructible_v<NameComponent>);

template <> struct entt::type_hash<NameComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3697051614;
  }
};
