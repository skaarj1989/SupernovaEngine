#pragma once

#include "AnimationLoader.hpp"
#include "entt/resource/cache.hpp"

using AnimationCache = entt::resource_cache<AnimationResource, AnimationLoader>;

class AnimationManager final : public AnimationCache {
public:
  [[nodiscard]] AnimationResourceHandle load(const std::filesystem::path &);
};
