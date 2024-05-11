#include "SkeletonCache.hpp"
#include "SkeletonInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, SkeletonCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](const auto id) {
        onDragSource<SkeletonCache::value_type>(
          id, [] { ImGui::TextUnformatted("Skeleton inside ..."); });
      },
      print, std::nullopt);
  }
  ImGui::End();
}
