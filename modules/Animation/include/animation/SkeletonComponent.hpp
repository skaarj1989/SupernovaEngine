#pragma once

#include "animation/SkeletonManager.hpp"

struct SkeletonComponent {
  std::shared_ptr<SkeletonResource> resource;

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(resource));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) resource = loadResource<SkeletonManager>(*path);
  }
};

static_assert(std::is_copy_constructible_v<SkeletonComponent>);
