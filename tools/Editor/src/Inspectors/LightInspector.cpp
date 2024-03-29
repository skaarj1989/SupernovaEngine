#include "Inspectors/LightInspector.hpp"
#include "renderer/Light.hpp"
#include "ImGuiHelper.hpp"
#include "glm/gtc/type_ptr.hpp" // value_ptr

void inspect(gfx::Light &light, const bool inspectTransform) {
  ImGui::PushItemWidth(150);

  ImGui::ComboEx("type", light.type,
                 "Directional\0"
                 "Spot\0"
                 "Point\0"
                 "\0");
  ImGui::Checkbox("visible", &light.visible);

  if (inspectTransform) {
    using enum gfx::LightType;

    if (light.type != Directional) {
      constexpr auto kTranslateSpeed = 0.1f;
      ImGui::DragFloat3("position", light.position, kTranslateSpeed, 0.0f, 0.0f,
                        "%.3f");
    }
    if (light.type != Point) {
      constexpr auto kRotateSpeed = 0.1f;
      ImGui::DragFloat3("direction", light.direction, kRotateSpeed, -1.0f, 1.0f,
                        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }
  }

  ImGui::Checkbox("castsShadow", &light.castsShadow);
  ImGui::BeginDisabled(!light.castsShadow);
  ImGui::SliderFloat("shadowBias", &light.shadowBias, 0.0f, 1.0f, "%.7f");
  ImGui::EndDisabled();

  ImGui::ColorEdit3("color", glm::value_ptr(light.color));
  ImGui::DragFloat("intensity", &light.intensity, 0.1f, 0.01f, 1000.0f, "%.3f",
                   ImGuiSliderFlags_AlwaysClamp);

  constexpr auto kMaxRange = 1000.0f;
  switch (light.type) {
    using enum gfx::LightType;

  case Spot: {
    ImGui::DragFloat("range", &light.range, 0.1f, 0.01f, kMaxRange, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat("innerConeAngle", &light.innerConeAngle, 0.1f, 0.01f,
                     light.outerConeAngle);
    constexpr auto kMaxOuterAngle = 80.0f;
    ImGui::DragFloat("outerConeAngle", &light.outerConeAngle, 0.1f,
                     light.innerConeAngle, kMaxOuterAngle, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
  } break;
  case Point:
    ImGui::DragFloat("radius", &light.range, 0.1f, 0.01f, kMaxRange, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
    break;

  default:
    break;
  }

  ImGui::Checkbox("debugVolume", &light.debugVolume);

  ImGui::PopItemWidth();
}
