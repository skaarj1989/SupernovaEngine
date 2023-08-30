#include "AnimationCache.hpp"
#include "Inspectors/AnimationInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, AnimationCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource(kImGuiPayloadTypeAnimation, id,
                     [] { ImGui::Text("Animation inside ..."); });
      },
      print, std::nullopt);
  }
  ImGui::End();
}
