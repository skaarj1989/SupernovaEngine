#pragma once

#include "ScriptLoader.hpp"
#include "entt/resource/cache.hpp"

using ScriptCache = entt::resource_cache<ScriptResource, ScriptLoader>;

class ScriptManager final : public ScriptCache {
public:
  [[nodiscard]] ScriptResourceHandle load(const std::filesystem::path &);
};
