#pragma once

#include "entt/core/type_info.hpp"
#include "entt/entity/entity.hpp"
#include <unordered_set>

struct ChildrenComponent {
  std::unordered_set<entt::entity> children;

  template <class Archive> void serialize(Archive &archive) {
    archive(children);
  }
};

template <> struct entt::type_hash<ChildrenComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 23910234;
  }
};
