#include "SceneEditor/SceneEditor.hpp"
#include "Inspectors/CharacterInspector.hpp"
#include "Inspectors/CharacterVirtualInspector.hpp"

void SceneEditor::_onInspect(entt::handle h, const Character &c) const {
  ImGui::Text("BodyID: %d", c.getBodyId().GetIndex());
  if (auto settings = c.getSettings(); inspect(settings)) {
    h.remove<Character>();
    h.emplace<Character>(settings);
  }
}
void SceneEditor::_onInspect(entt::handle h, CharacterVirtual &c) const {
  if (auto settings = c.getSettings(); inspect(settings)) {
    h.remove<CharacterVirtual>();
    h.emplace<CharacterVirtual>(settings);
  }
  if (auto b = c.isStickToFloor(); ImGui::Checkbox("stickToFloor", &b)) {
    c.setStickToFloor(b);
  }
  if (auto b = c.canWalkStairs(); ImGui::Checkbox("walkStairs", &b)) {
    c.setWalkStairs(b);
  }
}
