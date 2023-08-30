#pragma once

#include "MaterialResourceHandle.hpp"

namespace gfx {

struct MaterialLoader final : entt::resource_loader<MaterialResource> {
  result_type operator()(const std::filesystem::path &, TextureManager &) const;
  result_type operator()(const std::string_view, Material &&) const;
};

} // namespace gfx
