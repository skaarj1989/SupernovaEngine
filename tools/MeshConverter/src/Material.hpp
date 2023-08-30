#pragma once

#include "renderer/Material.hpp"
#include <set>

namespace offline {

struct Material {
  std::string name;

  gfx::ShadingModel shadingModel{gfx::ShadingModel::Lit};
  gfx::BlendMode blendMode{gfx::BlendMode::Opaque};
  gfx::LightingMode lightingMode{gfx::LightingMode::Default};
  bool twoSided{false};

  using Properties = std::unordered_map<std::string, gfx::Property::Value>;
  Properties properties;

  struct Texture {
    std::string path;
    uint32_t uvIndex{0};
  };
  using Textures = std::unordered_map<std::string, Texture>;
  Textures textures;

  std::set<std::string> userFragCodeDefines;
};

} // namespace offline
