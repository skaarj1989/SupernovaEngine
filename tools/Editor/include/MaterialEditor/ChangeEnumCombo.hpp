#pragma once

#include "imgui.h"
#include <type_traits>

template <typename T>
  requires std::is_scoped_enum_v<T>
[[nodiscard]] auto changeEnumCombo(const char *label, T &v,
                                   const char *(*nameGetter)(const T),
                                   ImGuiComboFlags flags) {
  auto changed = false;
  if (ImGui::BeginCombo(
        label, flags & ImGuiComboFlags_NoPreview ? nullptr : nameGetter(v),
        flags)) {
    constexpr auto kNumOptions = std::to_underlying(T::COUNT);
    for (auto i = 0; i < kNumOptions; ++i) {
      const auto option = static_cast<T>(i);
      auto selected = option == v;
      if (ImGui::Selectable(nameGetter(option), &selected,
                            selected ? ImGuiSelectableFlags_Disabled
                                     : ImGuiSelectableFlags_None)) {
        v = option;
        changed = true;
      }
      if (selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  return changed;
}
