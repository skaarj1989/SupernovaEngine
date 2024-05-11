#pragma once

#include "ImGuiPayloadHelper.hpp"

constexpr auto kImGuiPayloadTypeFile = "FILE";

namespace ImGui {

void ClearDragDrop(); // imgui_internal.h

inline void SetDragDropPayload(const char *type, const std::string_view s) {
  SetDragDropPayload(type, s.data(), s.length());
}

} // namespace ImGui

template <class T, typename Func>
void onDragSource(const entt::id_type id, Func onTooltipBody) {
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    static const auto *type = typeid(T).name();
    ImGui::SetDragDropPayload(type, &id, sizeof(entt::id_type));
    onTooltipBody();
    ImGui::EndDragDropSource();
  }
}

template <typename Func> void onDropTarget(const char *type, Func callback) {
  if (ImGui::BeginDragDropTarget()) {
    if (const auto *payload = ImGui::AcceptDragDropPayload(type); payload) {
      callback(payload);
    }
    ImGui::EndDragDropTarget();
  }
}

// Must be called between ImGui::{Begin/End}DragDropTarget().
template <class ResourceManager>
[[nodiscard]] auto
extractResourceFromPayload(ResourceManager &resourceManager) {
  using ResourceType =
    std::invoke_result_t<decltype(&ResourceManager::load), ResourceManager,
                         const std::filesystem::path &>;
  std::optional<ResourceType> incomingResource{std::nullopt};

  static const auto *type = typeid(typename ResourceManager::value_type).name();
  if (const auto *payload = ImGui::AcceptDragDropPayload(type); payload) {
    // 1. Cache -> Target control (text, image, whatever).
    const auto resourceId = ImGui::ExtractId(*payload);
    incomingResource = resourceManager[resourceId];
    assert(incomingResource);
  } else if (payload = ImGui::AcceptDragDropPayload(kImGuiPayloadTypeFile);
             payload) {
    // 2. File browser -> Target control.
    if (auto h = resourceManager.load(ImGui::ExtractPath(*payload)); h) {
      incomingResource = h;
    }
  }
  return incomingResource;
}
