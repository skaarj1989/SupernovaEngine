#pragma once

#include "ScriptResourceHandle.hpp"
#include "entt/resource/loader.hpp"

struct ScriptLoader final : entt::resource_loader<ScriptResource> {
  result_type operator()(const std::filesystem::path &) const;
  result_type operator()(std::string code) const {
    return std::make_shared<ScriptResource>(std::move(code), "");
  }
};
