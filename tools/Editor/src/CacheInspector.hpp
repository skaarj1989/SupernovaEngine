#pragma once

#include "os/FileSystem.hpp"
#include "ImGuiTitleBarMacro.hpp"
#include "IconsFontAwesome6.h"
#include "ImGuiModal.hpp"

template <class Cache> void defaultMenuBar(Cache &cache) {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Menu")) {
      if (ImGui::MenuItem(ICON_FA_ERASER " Clear")) {
        cache.clear();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

template <class Cache, typename OnDragCallback = std::monostate,
          typename TooltipCallback = std::monostate,
          typename IsRemovableCallback = std::monostate>
void view(Cache &cache, OnDragCallback onDrag, TooltipCallback tooltip,
          IsRemovableCallback isRemovable) {
  if (cache.empty()) {
    ImGui::Text("(empty)");
    return;
  }

  std::optional<entt::id_type> junk;

  constexpr auto kTableFlags =
    ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner |
    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags)) {
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Path");
    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoHeaderLabel);
    ImGui::TableHeadersRow();

    for (auto [id, resource] : cache) {
      assert(resource);

      ImGui::PushID(id);

      ImGui::TableNextRow();

      const auto path = resource->getPath();
      const auto filename = path.filename().string();

      ImGui::TableSetColumnIndex(0);
      ImGui::Selectable(filename.c_str());
      if constexpr (std::is_invocable_v<OnDragCallback, entt::id_type>) {
        onDrag(id);
      }

      if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();

        ImGui::BulletText("ID: %u", id);
        ImGui::PrintPath(path);

        using ResourceType = typename Cache::value_type;
        if constexpr (std::is_invocable_v<TooltipCallback,
                                          const ResourceType &>) {
          ImGui::Separator();
          tooltip(*resource);
        }

        ImGui::EndTooltip();
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::Text(resource->isVirtual()
                    ? "--"
                    : os::FileSystem::relativeToRoot(path)->string().c_str());

      ImGui::TableSetColumnIndex(2);

      constexpr auto kConfirmDestructionId = MAKE_WARNING("Remove");

      if constexpr (std::is_invocable_r_v<bool, IsRemovableCallback,
                                          entt::id_type>) {
        ImGui::BeginDisabled(!isRemovable(id));
      } else {
        ImGui::BeginDisabled(false);
      }
      if (ImGui::Button(ICON_FA_TRASH)) {
        ImGui::OpenPopup(kConfirmDestructionId);
      }
      ImGui::EndDisabled();
      if (const auto button =
            showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
              kConfirmDestructionId,
              "Do you really want to remove the resource?");
          button && *button == ModalButton::Yes) {
        junk = id;
      }

      ImGui::PopID();
    }

    ImGui::EndTable();

    if (junk) cache.erase(*junk);
  }
}
