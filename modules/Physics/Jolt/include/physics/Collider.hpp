#pragma once

#include "entt/core/type_info.hpp"
#include "ColliderManager.hpp"

struct ColliderComponent {
  std::shared_ptr<ColliderResource> resource;

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(resource));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) resource = loadResource<ColliderManager>(*path);
  }
};

static_assert(std::is_copy_constructible_v<ColliderComponent>);

template <> struct entt::type_hash<ColliderComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3929559123;
  }
};
