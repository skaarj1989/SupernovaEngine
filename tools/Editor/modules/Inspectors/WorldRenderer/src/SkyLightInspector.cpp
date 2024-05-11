#include "SkyLightInspector.hpp"
#include "Services.hpp"
#include "renderer/WorldRenderer.hpp"
#include "ResourceInspector.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "ImGuiPopups.hpp"

void inspect(gfx::SkyLight &skyLight, gfx::WorldRenderer &renderer) {
  print(skyLight.source.handle().get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource =
          extractResourceFromPayload(Services::Resources::Textures::value());
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
