#include "Inspectors/RigidBodyInspector.hpp"
#include "Inspectors/CollisionLayerInspector.hpp"
#include "ImGuiHelper.hpp"

bool inspect(RigidBody::Settings &settings) {
  auto dirty = false;

  dirty |= inspect(settings.layer);
  dirty |= ImGui::Checkbox("isSensor", &settings.isSensor);
  if (settings.motionType != MotionType::Static) {
    dirty |= ImGui::SliderFloat("mass", &settings.mass, 0.0f, 1000.0f);
  }
  dirty |= ImGui::ComboEx("motionType", settings.motionType,
                          "Dynamic\0"
                          "Static\0"
                          "Kinematic\0"
                          "\0");
  dirty |= ImGui::SliderFloat("friction", &settings.friction, 0.0f, 1.0f);
  dirty |= ImGui::SliderFloat("restitution", &settings.restitution, 0.0f, 1.0f);
  dirty |=
    ImGui::SliderFloat("linearDamping", &settings.linearDamping, 0.0f, 1.0f);
  dirty |=
    ImGui::SliderFloat("angularDamping", &settings.angularDamping, 0.0f, 1.0f);
  dirty |=
    ImGui::SliderFloat("gravityFactor", &settings.gravityFactor, 0.0f, 1.0f);

  return dirty;
}
