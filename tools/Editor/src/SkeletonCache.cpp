#include "SkeletonCache.hpp"
#include "Inspectors/SkeletonInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, SkeletonCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource(kImGuiPayloadTypeSkeleton, id,
                     [] { ImGui::TextUnformatted("Skeleton inside ..."); });
      },
      print, std::nullopt);
  }
  ImGui::End();
}
