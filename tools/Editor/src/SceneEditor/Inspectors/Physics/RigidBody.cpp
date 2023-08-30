#include "SceneEditor/SceneEditor.hpp"
#include "Inspectors/RigidBodyInspector.hpp"

void SceneEditor::_onInspect(entt::handle h, const RigidBody &c) const {
  if (auto settings = c.getSettings(); inspect(settings)) {
    h.remove<RigidBody>();
    h.emplace<RigidBody>(settings);
  }
}
