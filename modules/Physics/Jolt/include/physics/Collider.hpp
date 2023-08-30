#pragma once

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
