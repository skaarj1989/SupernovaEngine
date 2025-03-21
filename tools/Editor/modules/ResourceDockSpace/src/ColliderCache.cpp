#include "ColliderCache.hpp"
#include "ColliderInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, ColliderCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource<ColliderCache::value_type>(
          id, [] { ImGui::TextUnformatted("Collider inside ..."); });
      },
      [](const auto &r) { print(r.getCollisionShape()); }, std::nullopt);
  }
  ImGui::End();
}
