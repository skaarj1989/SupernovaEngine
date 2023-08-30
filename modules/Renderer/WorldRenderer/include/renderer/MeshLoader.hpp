#pragma once

#include "MeshResourceHandle.hpp"
#include "MaterialManager.hpp"

namespace gfx {

struct MeshLoader final : entt::resource_loader<MeshResource> {
  result_type operator()(const std::filesystem::path &, MaterialManager &,
                         rhi::RenderDevice &) const;
  result_type operator()(const std::string_view, Mesh &&) const;
};

} // namespace gfx
