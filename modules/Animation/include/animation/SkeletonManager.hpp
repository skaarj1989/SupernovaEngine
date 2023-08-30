#pragma once

#include "SkeletonLoader.hpp"
#include "entt/resource/cache.hpp"

using SkeletonCache = entt::resource_cache<SkeletonResource, SkeletonLoader>;

class SkeletonManager final : public SkeletonCache {
public:
  [[nodiscard]] SkeletonResourceHandle load(const std::filesystem::path &);
};
