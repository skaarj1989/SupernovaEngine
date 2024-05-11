#include "SceneEditor.hpp"
#include "TransformInspector.hpp"
#include "IconsFontAwesome6.h"

void SceneEditor::_onInspect(entt::handle, Transform &xf) {
  inspect(xf);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  const auto f = xf.getForward();
  ImGui::BulletText("Forward = [%.2f, %.2f, %.2f]", f.x, f.y, f.z);

  if (auto *entry = getActiveSceneEntry(); entry) {
    auto &camera = entry->viewport.camera;

    ImGui::SeparatorText("this");
    if (ImGui::Button(ICON_FA_LOCATION_ARROW " Pilot")) {
      xf.setOrientation(camera.getOrientation());
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROUTE " Summon")) {
      xf.setPosition(camera.getPosition());
    }

    ImGui::SeparatorText("viewport camera");
    if (ImGui::Button("Look at")) {
      auto temp = Transform{glm::inverse(camera.getView())};
      temp.lookAt(xf);
      camera.fromTransform(temp);
    }
    ImGui::SameLine();
    if (ImGui::Button("Move to")) {
      const auto q = camera.getOrientation();
      camera.fromTransform(xf).setOrientation(q);
    }
  }
}
