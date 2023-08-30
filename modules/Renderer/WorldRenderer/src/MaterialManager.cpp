#include "renderer/MaterialManager.hpp"
#include "entt/core/hashed_string.hpp"
#include "LoaderHelper.hpp"

namespace gfx {

namespace {

[[nodiscard]] auto createDefaultMaterial() {
  // https://stackoverflow.com/questions/4694608/glsl-checkerboard-pattern
  // clang-format off
  constexpr auto kFragCode =
R"(const float kCheckSize = 8.0;

const vec2 uv = floor(kCheckSize * getTexCoord0());
const vec3 pattern = vec3(max(sign(mod(uv.s + uv.t, 2.0)), 0.75));

#if HAS_PROPERTIES
material.baseColor.rgb = pattern * properties.baseColor.rgb;
material.metallic = properties.metallic;
material.roughness = properties.roughness;
#else
material.baseColor.rgb = pattern;
material.metallic = 0.0;
material.roughness = 0.6;
#endif
)";
  // clang-format on

  return Material::Builder{}
    .setSurface({
      .shadingModel = ShadingModel::Lit,
      .blendMode = BlendMode::Opaque,
      .cullMode = rhi::CullMode::Back,
    })
#if 1
    .addProperty({"baseColor", glm::vec4{1.0f}})
    .addProperty({"metallic", 0.04f})
    .addProperty({"roughness", 0.6f})
#endif
    .setUserFragCode({.source = kFragCode})
    .build();
}

} // namespace

//
// MaterialManager class:
//

const entt::hashed_string MaterialManager::kDefault = "Default";

MaterialManager::MaterialManager(TextureManager &textureManager)
    : m_textureManager{textureManager} {
  import(kDefault.data(), createDefaultMaterial());
}

bool MaterialManager::isBuiltIn(const std::filesystem::path &p) const {
  return isBuiltIn(entt::hashed_string{p.string().c_str()});
}
bool MaterialManager::isBuiltIn(entt::id_type id) const {
  return id == kDefault;
}

MaterialResourceHandle MaterialManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, isBuiltIn(p) ? LoadMode::BuiltIn : LoadMode::External,
                m_textureManager);
}
MaterialResourceHandle MaterialManager::import(const std::string_view name,
                                               Material &&material) {
  auto [it, _] = MaterialCache::load(entt::hashed_string{name.data()}.value(),
                                     name, std::move(material));
  return it->second;
}

void MaterialManager::clear() {
  const auto lastNotRemoved = std::remove_if(
    cbegin(), cend(), [](const auto it) { return it.first != kDefault; });
  erase(lastNotRemoved, end());
}

} // namespace gfx
