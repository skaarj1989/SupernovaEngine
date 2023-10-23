#pragma once

#include "ScopedEnumFlags.hpp"
#include "imgui.h"
#include "glm/ext/vector_float3.hpp"
#include <filesystem>
#include <utility>

namespace ImGui {

[[nodiscard]] ImGuiID GetLastItemID();

[[nodiscard]] glm::vec2 GetCurrentWindowCenter();

void CenterNextWindow(ImGuiCond = ImGuiCond_Appearing);

void DockBuilderDockWindow(const char *, ImGuiID); // imgui_internal.h

inline void DockBuilderDockWindows(std::initializer_list<const char *> names,
                                   ImGuiID nodeId) {
  for (const auto *name : names)
    DockBuilderDockWindow(name, nodeId);
}

void ShowTooltip(const std::string_view, ImVec2 padding = ImVec2{8, 8});

template <typename T> auto ComboEx(const char *label, T &v, const char *items) {
  auto currentItem = static_cast<int32_t>(v);
  const auto changed = Combo(label, &currentItem, items);
  if (changed) {
    v = static_cast<T>(currentItem);
  }
  return changed;
}

bool CheckboxN(bool *p_data, int32_t components);

template <ScopedEnumWithFlags T>
bool CheckboxFlags(const char *label, T &flags, T value) {
  return CheckboxFlags(label, std::bit_cast<int32_t *>(&flags),
                       std::to_underlying(value));
}

template <ScopedEnumWithFlags T>
bool MenuItemFlags(const char *label, T &flags, T value) {
  const auto enabled = (flags & value) == value;
  if (MenuItem(label, nullptr, enabled)) {
    if (enabled) {
      flags &= ~value;
    } else {
      flags |= value;
    }
    return true;
  }
  return false;
}

bool SliderAngle3(const char *label, glm::vec3 &);
bool InputFloat3(const char *label, glm::vec3 &, const char *format = "%.3f",
                 ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
bool DragFloat3(const char *label, glm::vec3 &, float speed = 1.0f,
                float min = 0.0f, float max = 0.0f, const char *format = "%.3f",
                ImGuiSliderFlags flags = ImGuiSliderFlags_None);

template <typename Func>
void Frame(Func callback, const ImVec2 padding = {4, 4}) {
  PushStyleVar(ImGuiStyleVar_CellPadding, padding);
  const auto visible = BeginTable("##FRAME", 1, ImGuiTableFlags_Borders);
  PopStyleVar();
  if (visible) {
    TableNextRow();
    TableSetColumnIndex(0);
    callback();
    EndTable();
  }
}

void PrintPath(const std::filesystem::path &);

} // namespace ImGui

int32_t blockFileSystemForbiddenCharacters(ImGuiInputTextCallbackData *);
