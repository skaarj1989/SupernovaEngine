#include "SceneEditor/SceneEditor.hpp"
#include "Services.hpp"

#include "Inspectors/MaterialInstanceInspector.hpp"
#include "Inspectors/ResourceInspector.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

#include <ranges>

namespace {

void onInspect(const int32_t index, gfx::SubMeshInstance &subMeshInstance) {
  const auto &material = subMeshInstance.material;
  const auto materialResource =
    std::dynamic_pointer_cast<Resource>(material.getPrototype());

  ImGui::Checkbox(IM_UNIQUE_ID, &subMeshInstance.visible);
  ImGui::SameLine();

  const auto treeOpened = ImGui::TreeNode(
    &subMeshInstance, ICON_FA_FILE " [%d] %s", index,
    materialResource ? toString(*materialResource).c_str() : "(none)");

  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeMaterial, Services::Resources::Materials::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); materialResource != r) {
        subMeshInstance.material = gfx::MaterialInstance{std::move(r)};
      }
    }
    ImGui::EndDragDropTarget();
  }

  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [&subMeshInstance] {
    const auto hasMaterial = bool(subMeshInstance.material);
    if (ImGui::MenuItem(ICON_FA_ERASER " Reset", nullptr, nullptr,
                        hasMaterial)) {
      subMeshInstance.material.reset();
    }
    if (ImGui::MenuItem(ICON_FA_TRASH " Remove", nullptr, nullptr,
                        hasMaterial)) {
      subMeshInstance.material = gfx::MaterialInstance{};
    }
  });

  if (treeOpened) {
    inspect(subMeshInstance.material);
    ImGui::SameLine();
    ImGui::Checkbox("Debug", &subMeshInstance.debug);
    ImGui::TreePop();
  }
}

template <class T> void onInspect(entt::handle h, T &instance) {
  const auto meshResource =
    std::dynamic_pointer_cast<Resource>(instance.getPrototype());

  print(meshResource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeMesh, Services::Resources::Meshes::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); meshResource != r) {
        h.replace<T>(r);
      }
    }
    ImGui::EndDragDropTarget();
  }
  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right,
              [&h, &instance, hasResource = bool(meshResource)] {
                if (ImGui::MenuItem(ICON_FA_ERASER " Reset", nullptr, nullptr,
                                    hasResource)) {
                  instance.reset();
                }
                if (ImGui::MenuItem(ICON_FA_TRASH " Remove", nullptr, nullptr,
                                    hasResource)) {
                  h.replace<T>();
                }
              });

  if (instance) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    for (auto [i, subMesh] : std::views::enumerate(instance.each())) {
      ImGui::PushID(i);
      onInspect(i, subMesh);
      ImGui::PopID();
      ++i;
    }
  }
}

} // namespace

void SceneEditor::_onInspect(entt::handle h,
                             gfx::MeshInstance &meshInstance) const {
  onInspect(h, meshInstance);
}
void SceneEditor::_onInspect(entt::handle h,
                             gfx::DecalInstance &decalInstance) const {
  onInspect(h, decalInstance);
}
