#include "SceneEditor.hpp"
#include "Services.hpp"

#include "ResourceInspector.hpp"
#include "AudioComponentInspector.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

void SceneEditor::_onInspect(entt::handle, SoundSourceComponent &c) const {
  const auto resource = c.getClip();
  print(resource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource =
          extractResourceFromPayload(Services::Resources::AudioClips::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); resource != r) {
        c.setClip(r);
      }
    }
    ImGui::EndDragDropTarget();
  }

  if (resource) {
    attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [&c] {
      if (ImGui::MenuItem(ICON_FA_ERASER " Remove")) c.setClip(nullptr);
    });
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  inspect(c);
}
