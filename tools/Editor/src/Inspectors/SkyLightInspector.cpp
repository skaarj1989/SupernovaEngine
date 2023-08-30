#include "Inspectors/SkyLightInspector.hpp"
#include "Inspectors/ResourceInspector.hpp"

#include "Services.hpp"

#include "ImGuiDragAndDrop.hpp"
#include "ImGuiPopups.hpp"

void inspect(gfx::SkyLight &skyLight, gfx::WorldRenderer &renderer) {
  print(skyLight.source.handle().get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeTexture, Services::Resources::Textures::value());
        incomingResource) {
      if (skyLight.source != *incomingResource) {
        skyLight = renderer.createSkyLight(*incomingResource);
      }
    }
    ImGui::EndDragDropTarget();
  }
  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [&skyLight] {
    if (ImGui::MenuItem("Remove", nullptr, nullptr, bool(skyLight))) {
      skyLight = gfx::SkyLight{};
    }
  });
}
