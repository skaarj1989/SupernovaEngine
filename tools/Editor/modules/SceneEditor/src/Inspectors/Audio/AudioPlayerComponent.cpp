#include "SceneEditor.hpp"
#include "AudioComponentInspector.hpp"

void SceneEditor::_onInspect(entt::handle, AudioPlayerComponent &c) const {
  inspect(c);
}
