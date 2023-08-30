#include "SceneEditor/SceneEditor.hpp"
#include "Inspectors/CharacterInspector.hpp"

void SceneEditor::_onInspect(entt::handle h, const Character &c) const {
  if (auto settings = c.getSettings(); inspect(settings)) {
    h.remove<Character>();
    h.emplace<Character>(settings);
  }
}
