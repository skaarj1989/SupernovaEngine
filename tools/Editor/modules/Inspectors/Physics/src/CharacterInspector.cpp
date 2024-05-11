#include "CharacterInspector.hpp"
#include "CollisionLayerInspector.hpp"
#include "imgui.h"
#include "glm/trigonometric.hpp" // radians, degrees

bool inspect(Character::Settings &settings) {
  auto dirty = false;

  ImGui::PushItemWidth(150.0f);

  if (auto maxSlopeAngle = glm::radians(settings.maxSlopeAngle);
      ImGui::SliderAngle("maxSlopeAngle", &maxSlopeAngle, 0.0f, 180.0f)) {
    settings.maxSlopeAngle = glm::degrees(maxSlopeAngle);
    dirty = true;
  }
  dirty |= inspect(settings.layer);
  dirty |= ImGui::SliderFloat("mass", &settings.mass, 0.1f, 1000.0f);
  dirty |= ImGui::SliderFloat("friction", &settings.friction, 0.0f, 1.0f);
  dirty |=
    ImGui::SliderFloat("gravityFactor", &settings.gravityFactor, 0.0f, 1.0f);

  ImGui::PopItemWidth();

  return dirty;
}
