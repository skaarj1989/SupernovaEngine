#include "SceneEditor/SceneEditor.hpp"
#include "Services.hpp"

#include "Inspectors/ResourceInspector.hpp"
#include "Inspectors/ColliderInspector.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

void SceneEditor::_onInspect(entt::handle h, const ColliderComponent &c) const {
  const auto &[resource] = c;

  print(resource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeCollider, Services::Resources::Colliders::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); resource != r) {
        h.replace<ColliderComponent>(r);
      }
    }
    ImGui::EndDragDropTarget();
  }

  if (resource) {
    attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [h] {
      if (ImGui::MenuItem(ICON_FA_ERASER " Remove"))
        h.replace<ColliderComponent>();
    });
  }
  if (resource) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    print(resource->getCollisionShape());
  }
}
