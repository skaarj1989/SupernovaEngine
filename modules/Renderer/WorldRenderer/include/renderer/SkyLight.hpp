#pragma once

#include "TextureManager.hpp"

namespace gfx {

struct SkyLight {
  explicit SkyLight(TextureResourceHandle h = {}) : source{h} {}

  TextureResourceHandle source;
  std::shared_ptr<rhi::Texture> environment;

  std::shared_ptr<rhi::Texture> diffuse;
  std::shared_ptr<rhi::Texture> specular;

  [[nodiscard]] explicit operator bool() const;

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(source.handle()));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) source = loadResourceHandle<TextureManager>(*path);
  }
};

} // namespace gfx
