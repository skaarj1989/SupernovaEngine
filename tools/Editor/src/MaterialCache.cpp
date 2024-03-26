#include "MaterialCache.hpp"
#include "renderer/MaterialManager.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

namespace {

void print(const gfx::Material::Surface &surface) {
  ImGui::BulletText("ShadingModel: %s", gfx::toString(surface.shadingModel));
  ImGui::BulletText("BlendMode: %s", gfx::toString(surface.blendMode));

  // TODO: Decal blend modes.

  ImGui::BulletText("LightingMode: %s", gfx::toString(surface.lightingMode));
  ImGui::BulletText("CullMode: %s", toString(surface.cullMode));
}
void print(const gfx::Material::Blueprint &blueprint) {
  if (blueprint.surface) {
    ImGui::BulletText("Domain: Surface");

    print(*blueprint.surface);

    ImGui::Separator();

    ImGui::BulletText("Num properties: %zu", blueprint.properties.size());
    ImGui::BulletText("Num textures: %zu", blueprint.defaultTextures.size());
  } else {
    ImGui::BulletText("Domain: PostProcess");
  }
}

} // namespace

void show(const char *name, bool *open, gfx::MaterialManager &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](const auto id) {
        onDragSource(kImGuiPayloadTypeMaterial, id,
                     [] { ImGui::TextUnformatted("Material inside ..."); });
      },
      [](const auto &r) { print(r.getBlueprint()); },
      [&cache](const auto id) { return !cache.isBuiltIn(id); });
  }
  ImGui::End();
}
