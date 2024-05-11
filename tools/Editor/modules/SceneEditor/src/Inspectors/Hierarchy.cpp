#include "SceneEditor.hpp"
#include "IconsFontAwesome6.h"

void SceneEditor::_onInspect(entt::handle h, const ParentComponent &c) {
  if (entt::null != c.parent) {
    if (ImGui::BeginTable(IM_UNIQUE_ID, 2)) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::AlignTextToFramePadding();
      const auto label = std::format("{} Parent: {}", ICON_FA_PERSON,
                                     entt::to_integral(c.parent));
      if (ImGui::Selectable(label.c_str())) {
        m_dispatcher.enqueue(
          SelectEntityRequest{entt::handle{*h.registry(), c.parent}});
      }

      ImGui::TableSetColumnIndex(1);
      if (ImGui::Button(ICON_FA_LINK_SLASH " Detach")) {
        m_dispatcher.enqueue(DetachEntityRequest{h});
      }

      ImGui::EndTable();
    }
  } else {
    ImGui::TextUnformatted("(none)");
  }
}
void SceneEditor::_onInspect(entt::handle h, const ChildrenComponent &c) {
  if (!c.children.empty()) {
    for (const auto e : c.children) {
      const auto label =
        std::format("{} {}", ICON_FA_CHILD, entt::to_integral(e));
      if (ImGui::Selectable(label.c_str())) {
        m_dispatcher.enqueue(
          SelectEntityRequest{entt::handle{*h.registry(), e}});
      }
    }
  } else {
    ImGui::TextUnformatted("(empty)");
  }
}
