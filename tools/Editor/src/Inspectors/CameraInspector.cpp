#include "Inspectors/CameraInspector.hpp"
#include "Inspectors/TransformInspector.hpp"
#include "imgui.h"

void inspect(gfx::PerspectiveCamera &camera, bool inspectTransform) {
  ImGui::PushItemWidth(100);

  if (auto fov = camera.getFov();
      ImGui::SliderFloat(IM_UNIQUE_ID, &fov, 35.0f, 120.0f, "fov = %.3f")) {
    camera.setFov(fov);
  }
  ImGui::Text("Aspect ratio = %.2f", camera.getAspectRatio());

  auto [zNear, zFar] = camera.getClippingPlanes();
  bool dirtyClipPlanes{false};

  dirtyClipPlanes |=
    ImGui::SliderFloat(IM_UNIQUE_ID, &zNear, 0.01f, 1.0f, "near = %.3f");
  ImGui::SameLine();
  dirtyClipPlanes |=
    ImGui::SliderFloat(IM_UNIQUE_ID, &zFar, zNear, 1000.0f, "far = %.3f");
  ImGui::PopItemWidth();
  if (dirtyClipPlanes) camera.setClippingPlanes({zNear, zFar});

  if (auto b = camera.isFrustumFreezed();
      ImGui::Checkbox("Freeze frustum", &b)) {
    camera.freezeFrustum(b);
  }

  if (inspectTransform) {
    ImGui::SeparatorText("Transform");
    if (auto v = camera.getPosition(); inspectPosition(v)) {
      camera.setPosition(v);
    }
    if (auto q = camera.getOrientation(); inspectOrientation(q)) {
      camera.setOrientation(q);
    }
    const auto v = camera.getForward();
    ImGui::Text("Forward = [%.2f, %.2f, %.2f]", v.x, v.y, v.z);
  }
}
