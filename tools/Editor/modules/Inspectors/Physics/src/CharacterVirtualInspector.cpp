#include "CharacterVirtualInspector.hpp"
#include "CollisionLayerInspector.hpp"
#include "ImGuiHelper.hpp"
#include "glm/trigonometric.hpp" // radians, degrees

bool inspect(CharacterVirtual::Settings &settings) {
  auto dirty = false;

  ImGui::PushItemWidth(150.0f);

  if (auto maxSlopeAngle = glm::radians(settings.maxSlopeAngle);
      ImGui::SliderAngle("maxSlopeAngle", &maxSlopeAngle, 0.0f, 180.0f)) {
    settings.maxSlopeAngle = glm::degrees(maxSlopeAngle);
    dirty = true;
  }
  dirty |= inspect(settings.layer);
  dirty |= ImGui::SliderFloat("mass", &settings.mass, 0.01f, 1000.0f);

  dirty |=
    ImGui::SliderFloat("maxStrength", &settings.maxStrength, 0.01f, 1000.0f);

  dirty |= ImGui::DragFloat3("shapeOffset", settings.shapeOffset);
  dirty |=
    ImGui::SliderFloat("predictiveContactDistance",
                       &settings.predictiveContactDistance, 0.001f, 100.0f);
  const std::pair kMaxCollisionIterationsRange{1u, 10u};
  dirty |= ImGui::SliderScalar("maxCollisionIterations", ImGuiDataType_U32,
                               &settings.maxCollisionIterations,
                               &kMaxCollisionIterationsRange.first,
                               &kMaxCollisionIterationsRange.second);
  const std::pair kMaxConstraintIterationsRange{1u, 15u};
  dirty |= ImGui::SliderScalar("maxConstraintIterations", ImGuiDataType_U32,
                               &settings.maxConstraintIterations,
                               &kMaxConstraintIterationsRange.first,
                               &kMaxConstraintIterationsRange.second);
  dirty |= ImGui::SliderFloat("minTimeRemaining", &settings.minTimeRemaining,
                              0.0f, 10.0f);
  dirty |= ImGui::SliderFloat("collisionTolerance",
                              &settings.collisionTolerance, 0.0f, 10.0f);
  dirty |= ImGui::SliderFloat("characterPadding", &settings.characterPadding,
                              0.0f, 10.0f);
  const std::pair kMaxNumHitsRange{0u, 1000u};
  dirty |=
    ImGui::SliderScalar("maxNumHits", ImGuiDataType_U32, &settings.maxNumHits,
                        &kMaxNumHitsRange.first, &kMaxNumHitsRange.second);
  dirty |= ImGui::SliderFloat("hitReductionCosMaxAngle",
                              &settings.hitReductionCosMaxAngle, -1.0f, 10.0f);
  dirty |= ImGui::SliderFloat("penetrationRecoverySpeed",
                              &settings.penetrationRecoverySpeed, 0.0f, 1.0f);

  ImGui::PopItemWidth();

  return dirty;
}
