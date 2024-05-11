#pragma once

#include "ImGuiHelper.hpp" // GetLastItemID

namespace ImGui {

bool BeginPopupEx(ImGuiID, ImGuiWindowFlags); // imgui_internal.h

} // namespace ImGui

template <typename Func>
void attachPopup(const char *id, const ImGuiMouseButton mouseButton,
                 Func callback) {
  constexpr auto kPopupFlags = ImGuiWindowFlags_NoDecoration |
                               ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoMove;
  ImGui::OpenPopupOnItemClick(id,
                              ImGuiPopupFlags_MouseButtonMask_ & mouseButton);
  if (id ? ImGui::BeginPopup(id, kPopupFlags)
         : ImGui::BeginPopupEx(ImGui::GetLastItemID(), kPopupFlags)) {
    callback();
    ImGui::EndPopup();
  }
}
