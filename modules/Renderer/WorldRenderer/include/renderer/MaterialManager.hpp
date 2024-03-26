#pragma once

#include "MaterialLoader.hpp"
#include "entt/core/hashed_string.hpp"

namespace gfx {

using MaterialCache = entt::resource_cache<MaterialResource, MaterialLoader>;

class MaterialManager final : public MaterialCache {
public:
  explicit MaterialManager(TextureManager &);

  [[nodiscard]] bool isBuiltIn(const std::filesystem::path &p) const;
  [[nodiscard]] bool isBuiltIn(const entt::id_type) const;

  [[nodiscard]] MaterialResourceHandle load(const std::filesystem::path &);
  MaterialResourceHandle import(const std::string_view name, Material &&);

  static const entt::hashed_string kDefault;

  void clear();

private:
  TextureManager &m_textureManager;
};

} // namespace gfx
