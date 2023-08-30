#include "SceneEditor/SceneEditor.hpp"
#include "Services.hpp"
#include "Inspectors/ResourceInspector.hpp"
#include "Inspectors/AnimationInspector.hpp"
#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

void SceneEditor::_onInspect(entt::handle,
                             AnimationComponent &animationComponent) const {
  auto &[resource] = animationComponent;

  print(resource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeAnimation, Services::Resources::Animations::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); resource != r) {
        resource = std::move(r);
      }
    }
    ImGui::EndDragDropTarget();
  }

  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [&resource] {
    if (ImGui::MenuItem(ICON_FA_ERASER " Remove", nullptr, nullptr,
                        resource != nullptr)) {
      resource = {};
    }
  });

  if (resource) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    print(*resource);
  }
}
