#include "AnimationCache.hpp"
#include "AnimationInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, AnimationCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](const auto id) {
        onDragSource<AnimationCache::value_type>(
          id, [] { ImGui::TextUnformatted("Animation inside ..."); });
      },
      print, std::nullopt);
  }
  ImGui::End();
}
