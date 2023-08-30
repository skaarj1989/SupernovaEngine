#pragma once

#include "ColliderLoader.hpp"
#include "entt/resource/cache.hpp"

using ColliderCache = entt::resource_cache<ColliderResource, ColliderLoader>;

class ColliderManager final : public ColliderCache {
public:
  [[nodiscard]] ColliderResourceHandle load(const std::filesystem::path &);
};
