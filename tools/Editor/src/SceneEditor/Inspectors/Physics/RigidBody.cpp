#include "SceneEditor/SceneEditor.hpp"
#include "Inspectors/RigidBodyInspector.hpp"

void SceneEditor::_onInspect(entt::handle h, const RigidBody &rb) const {
  ImGui::Text("BodyID: %d", rb.getBodyId().GetIndex());
  if (auto settings = rb.getSettings(); inspect(settings)) {
    h.remove<RigidBody>();
    h.emplace<RigidBody>(settings);
  }
}
