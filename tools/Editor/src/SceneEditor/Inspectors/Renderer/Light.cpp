#include "SceneEditor/SceneEditor.hpp"
#include "Inspectors/LightInspector.hpp"

void SceneEditor::_onInspect(entt::handle, gfx::Light &light) const {
  inspect(light, false);
}
