#pragma once

#include "Resource.hpp"
#include "rhi/Texture.hpp"

namespace gfx {

class TextureResource final : public Resource, public rhi::Texture {
public:
  TextureResource() = default;
  TextureResource(rhi::Texture &&, const std::filesystem::path &);
  TextureResource(const TextureResource &) = delete;
  TextureResource(TextureResource &&) noexcept = default;

  TextureResource &operator=(const TextureResource &) = delete;
  TextureResource &operator=(TextureResource &&) noexcept = default;
};

} // namespace gfx
