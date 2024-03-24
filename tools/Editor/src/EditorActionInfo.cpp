#include "EditorActionInfo.hpp"
#include "imgui.h"

void keyboardShortcutsTable(const char *label,
                            std::span<const EditorActionInfo> entries) {
  ImGui::SeparatorText(label);
  if (ImGui::BeginTable(IM_UNIQUE_ID, 2,
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                          ImGuiTableFlags_SizingStretchProp)) {
    for (const auto &[description, shortcuts] : entries) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      for (const auto *chord : shortcuts) {
        ImGui::TextUnformatted(chord);
      }
      ImGui::TableSetColumnIndex(1);
      ImGui::TextUnformatted(description);
    }
    ImGui::EndTable();
  }
}
