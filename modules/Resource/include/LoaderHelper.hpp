#pragma once

#include "os/FileSystem.hpp"
#include "spdlog/spdlog.h"

enum class LoadMode { External, BuiltIn };

template <typename Type, typename Loader, typename... Args>
[[nodiscard]] auto load(entt::resource_cache<Type, Loader> &c,
                        std::filesystem::path p, LoadMode loadMode,
                        Args &&...args) -> entt::resource<Type> {
  if (loadMode == LoadMode::External) {
    p = os::FileSystem::getRoot() / p;
  }
  auto [it, emplaced] =
    c.load(makeResourceId(p), p, std::forward<Args>(args)...);
  if (!it->second) {
    c.erase(it);
    return {};
  }
  if (emplaced) {
    SPDLOG_INFO("Loaded resource: {}",
                os::FileSystem::relativeToRoot(p)->string());
  }
  return it->second;
}
