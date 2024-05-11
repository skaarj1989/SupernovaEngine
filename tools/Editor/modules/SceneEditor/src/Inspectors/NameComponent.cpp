#include "SceneEditor.hpp"
#include "imgui_stdlib.h" // InputText

void SceneEditor::_onInspect(entt::handle, NameComponent &component) const {
  ImGui::InputText(IM_UNIQUE_ID, &component.name);
}
