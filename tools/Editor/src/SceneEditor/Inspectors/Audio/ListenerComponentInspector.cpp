#include "SceneEditor/SceneEditor.hpp"

void SceneEditor::_onInspect(entt::handle h,
                             [[maybe_unused]] ListenerComponent &) const {
  auto &[mainListener] = getMainListener(*h.registry());
  if (auto b = mainListener == h; ImGui::Checkbox("main", &b)) {
    mainListener = b ? h.entity() : entt::null;
  }
}
