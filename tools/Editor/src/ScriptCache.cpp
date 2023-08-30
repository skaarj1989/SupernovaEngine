#include "ScriptCache.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

void show(const char *name, bool *open, ScriptCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource(kImGuiPayloadTypeScript, id,
                     [] { ImGui::Text("Script inside ..."); });
      },
      std::nullopt, std::nullopt);
  }
  ImGui::End();
}
