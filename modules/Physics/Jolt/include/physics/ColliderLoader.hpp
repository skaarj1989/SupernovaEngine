#pragma once

#include "ColliderResourceHandle.hpp"
#include "entt/resource/loader.hpp"

struct ColliderLoader final : entt::resource_loader<ColliderResource> {
  result_type operator()(const std::filesystem::path &) const;
};
