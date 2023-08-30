#include "SceneEditor/SceneEditor.hpp"
#include "Services.hpp"
#include "Inspectors/ResourceInspector.hpp"
#include "Inspectors/SkeletonInspector.hpp"
#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

void SceneEditor::_onInspect(entt::handle,
                             SkeletonComponent &skeletonComponent) const {
  auto &[resource] = skeletonComponent;

  print(resource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeSkeleton, Services::Resources::Skeletons::value());
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

  if (resource) print(*resource);
}
