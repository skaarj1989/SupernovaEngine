#pragma once

#include "SkeletonResourceHandle.hpp"
#include "entt/resource/loader.hpp"

struct SkeletonLoader final : entt::resource_loader<SkeletonResource> {
  result_type operator()(const std::filesystem::path &) const;
};
