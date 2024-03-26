#pragma once

#include "TextureManager.hpp"
#include <map>

namespace gfx {

struct TextureInfo {
  rhi::TextureType type{rhi::TextureType::Undefined};
  std::shared_ptr<rhi::Texture> texture;

  [[nodiscard]] auto isValid() const {
    return type != rhi::TextureType::Undefined && texture && *texture;
  }

  friend bool operator==(const TextureInfo &a, const TextureInfo &b) {
    return a.texture == b.texture;
  }

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(texture));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) {
      texture = loadResource<TextureManager>(*path);
      if (texture) type = texture->getType();
    }
  }
};
using TextureResources = std::map<std::string, TextureInfo, std::less<>>;

[[nodiscard]] bool operator==(const TextureResources &,
                              const TextureResources &);

} // namespace gfx
