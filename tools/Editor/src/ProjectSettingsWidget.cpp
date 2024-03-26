#include "ProjectSettingsWidget.hpp"
#include "ProjectSettings.hpp"
#include "ImGuiModal.hpp"
#include "imgui_stdlib.h" // InputText

namespace {

void inspect(ProjectSettings &settings) {
  constexpr auto kTableFlags =
    ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_SizingStretchProp;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 2, kTableFlags)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Path");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    ImGui::SetNextItemWidth(100);
    if (ImGui::InputText(IM_UNIQUE_ID, &settings.name,
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
      settings.dirty = true;
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::TextUnformatted(
      settings.path.parent_path().generic_string().c_str());

    ImGui::EndTable();
  }
}

} // namespace

void showProjectSettings(const char *name, ProjectSettings &settings) {
  showModal<ModalButtons::Cancel>(name, [&settings] { inspect(settings); });
}
