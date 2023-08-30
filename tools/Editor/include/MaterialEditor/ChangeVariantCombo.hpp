#pragma once

#include "imgui.h"
#include <optional>
#include <variant>
#include <span>

// nullopt = Separator
template <typename T> using Option = std::optional<std::pair<const char *, T>>;

template <typename T>
[[nodiscard]] bool changeVariantCombo(const char *label, T &variant,
                                      std::span<const Option<T>> options) {
  auto changed = false;
  if (ImGui::BeginCombo(label, nullptr, ImGuiComboFlags_NoPreview)) {
    for (const auto &option : options) {
      if (option) {
        const auto &[name, defaultValue] = *option;
        auto selected = defaultValue.index() == variant.index();
        if (ImGui::Selectable(name, &selected,
                              selected ? ImGuiSelectableFlags_Disabled
                                       : ImGuiSelectableFlags_None)) {
          variant = defaultValue;
          changed = true;
        }
        if (selected) ImGui::SetItemDefaultFocus();
      } else {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
      }
    }
    ImGui::EndCombo();
  }
  return changed;
}
