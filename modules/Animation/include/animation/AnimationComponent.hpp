#pragma once

#include "animation/AnimationManager.hpp"

struct AnimationComponent {
  std::shared_ptr<AnimationResource> resource;

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(resource));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) resource = loadResource<AnimationManager>(*path);
  }
};

static_assert(std::is_copy_constructible_v<AnimationComponent>);
