#pragma once

#include "AnimationResourceHandle.hpp"
#include "entt/resource/loader.hpp"

struct AnimationLoader final : entt::resource_loader<AnimationResource> {
  result_type operator()(const std::filesystem::path &) const;
};
