#pragma once

#include "TextureResourceHandle.hpp"
#include "entt/resource/loader.hpp"

namespace gfx {

struct TextureLoader final : entt::resource_loader<TextureResource> {
  result_type operator()(const std::filesystem::path &,
                         rhi::RenderDevice &) const;
  result_type operator()(rhi::Texture &&) const;
};

} // namespace gfx
