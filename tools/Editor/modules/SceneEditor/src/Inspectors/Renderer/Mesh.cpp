#include "SceneEditor.hpp"
#include "Services.hpp"

#include "MaterialInstanceInspector.hpp"
#include "ResourceInspector.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp"
#include "ImGuiDragAndDrop.hpp"

#include <ranges>

void setOutline(gfx::MeshInstance *mi, const bool selected) {
  if (mi) {
    for (auto &sm : mi->each()) {
      if (selected) {
        sm.flags |= gfx::SubMeshInstance::Flags::ShowOutline;
      } else {
        sm.flags &= ~gfx::SubMeshInstance::Flags::ShowOutline;
      }
    }
  }
}

namespace {

void onInspect(const uint32_t index, gfx::SubMeshInstance &subMeshInstance) {
  const auto &material = subMeshInstance.material;
  const auto materialResource =
    std::dynamic_pointer_cast<Resource>(material.getPrototype());

  ImGui::Checkbox(IM_UNIQUE_ID, &subMeshInstance.visible);
  ImGui::SameLine();

  const auto treeOpened = ImGui::TreeNode(
    &subMeshInstance, ICON_FA_FILE " [%u] %s", index,
    materialResource ? toString(*materialResource).c_str() : "(none)");

  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource =
          extractResourceFromPayload(Services::Resources::Materials::value());
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
    ImGui::CheckboxFlags("AABB", subMeshInstance.flags,
                         gfx::SubMeshInstance::Flags::ShowAABB);
    ImGui::TreePop();
  }
}

template <class T> void onInspect(const entt::handle h, T &instance) {
  const auto meshResource =
    std::dynamic_pointer_cast<Resource>(instance.getPrototype());

  print(meshResource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource =
          extractResourceFromPayload(Services::Resources::Meshes::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); meshResource != r) {
        h.replace<T>(r);
        setOutline(&instance, true);
      }
    }
    ImGui::EndDragDropTarget();
  }
  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right,
              [&h, &instance, hasResource = bool(meshResource)] {
                if (ImGui::MenuItem(ICON_FA_ERASER " Reset", nullptr, nullptr,
                                    hasResource)) {
                  instance.reset();
                  setOutline(&instance, true);
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

    for (auto [i, subMesh] : instance.each() | std::views::enumerate) {
      ImGui::PushID(static_cast<int32_t>(i));
      onInspect(static_cast<uint32_t>(i), subMesh);
      ImGui::PopID();
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
