#include "SceneEditor.hpp"
#include "LightInspector.hpp"

void SceneEditor::_onInspect(entt::handle, gfx::Light &light) const {
  inspect(light, false);
}
