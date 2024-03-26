#include "Inspectors/TransformInspector.hpp"
#include "Transform.hpp"
#include "ImGuiHelper.hpp"
#include "glm/gtc/quaternion.hpp" // eulerAnles

bool inspect(Transform &xf) {
  auto dirty = false;
  if (auto v = xf.getLocalPosition(); inspectPosition(v)) {
    xf.setPosition(v);
    dirty = true;
  }
  if (auto q = xf.getOrientation(); inspectOrientation(q)) {
    xf.setOrientation(q);
    dirty = true;
  }
  if (auto v = xf.getLocalScale(); inspectScale(v)) {
    xf.setScale(v);
    dirty = true;
  }
  return dirty;
}

bool inspectPosition(glm::vec3 &v) {
  constexpr auto kSpeed = 0.1f;
  return ImGui::DragFloat3("position", v, kSpeed);
}
bool inspectOrientation(glm::quat &q) {
  if (auto v = glm::degrees(glm::eulerAngles(q));
      ImGui::InputFloat3("orientation", v)) {
    constexpr auto kMaxAngle = 360.0f;
    v = glm::clamp(v, -kMaxAngle, kMaxAngle);
    q = glm::radians(v);
    return true;
  }
  return false;
}
bool inspectScale(glm::vec3 &v) {
  constexpr auto kSpeed = 0.1f;
  return ImGui::DragFloat3("scale", v, kSpeed, FLT_EPSILON, FLT_MAX, "%.3f",
                           ImGuiSliderFlags_AlwaysClamp);
}
