#include "CameraController.hpp"
#include "PerspectiveCamera.hpp"
#include "imgui.h"
#include "glm/gtx/quaternion.hpp" // normalize, angleAxis
#include "glm/gtx/transform.inl"  // rotate

namespace {

enum class Operation { Zoom, Rotate, Pan };

enum OperationMask_ {
  OperationMask_None = 0,

  OperationMask_Zoom = 1 << 0,
  OperationMask_Rotate = 1 << 1,
  OperationMask_Pan = 1 << 2,

  OperationMask_All =
    OperationMask_Zoom | OperationMask_Rotate | OperationMask_Pan
};
using OperationMask = int32_t;

[[nodiscard]] std::optional<Operation>
controllerBehavior(OperationMask mask = OperationMask_All) {
  if (ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
      ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    if (mask & OperationMask_Zoom) return Operation::Zoom;
  } else {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      if (mask & OperationMask_Rotate) return Operation::Rotate;
    } else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
      if (mask & OperationMask_Pan) return Operation::Pan;
    }
  }
  return std::nullopt;
}

void zoom(gfx::PerspectiveCamera &camera, glm::vec2 mouseDelta, float speed) {
  auto p = camera.getPosition();
  const auto forward = camera.getForward();
  p -= (forward * (mouseDelta.x * speed));
  p += (forward * (mouseDelta.y * speed));
  camera.setPosition(p);
}

void arcball(gfx::PerspectiveCamera &camera,
             const CameraController::ArcBallConfig &config,
             glm::vec2 mouseDelta, float speed) {
  const auto deltaAngle =
    glm::vec2{
      glm::two_pi<float>(), // A movement from left to right = 2*PI = 360 deg.
      glm::pi<float>(),     // A movement from top to bottom = PI = 180 deg.
    } /
    config.viewport;
  const auto angles = mouseDelta * deltaAngle * speed;

  const glm::vec4 pivot{config.pivot, 1.0f};
  auto position = glm::vec4{camera.getPosition(), 1.0f};
  const auto rotate = [&](float angle, glm::vec3 v) {
    position = glm::rotate(angle, v) * (position - pivot) + pivot;
  };

  rotate(angles.y, camera.getRight());
  rotate(-angles.x, camera.getUp());

  camera.fromTransform(Transform{}.setPosition(position).lookAt(pivot));
}
void rotate(gfx::PerspectiveCamera &camera, glm::vec2 mouseDelta, float speed) {
  const auto yaw = -mouseDelta.x * speed;
  auto qYaw = glm::angleAxis(glm::radians(yaw), Transform::kUp);

  auto q = camera.getOrientation();
  q = glm::normalize(qYaw * q);

  const auto pitch = mouseDelta.y * speed;
  auto qPitch = glm::angleAxis(glm::radians(pitch), Transform::kRight);
  q = glm::normalize(q * qPitch);

  camera.setOrientation(q);
}

void pan(gfx::PerspectiveCamera &camera, glm::vec2 mouseDelta, float speed) {
  auto p = camera.getPosition();
  p += camera.getRight() * (mouseDelta.x * speed);
  p += camera.getUp() * (mouseDelta.y * speed);
  camera.setPosition(p);
}

} // namespace

CameraController::Result
CameraController::update(gfx::PerspectiveCamera &camera,
                         const Settings &settings, glm::vec2 mouseDelta,
                         std::optional<ArcBallConfig> arcballConfig) {
  const auto op =
    controllerBehavior(arcballConfig ? OperationMask_Zoom | OperationMask_Rotate
                                     : OperationMask_All);
  Result result{.wantUse = op.has_value()};
  if (result.wantUse && glm::length(mouseDelta) > 0.0f) {
    switch (*op) {
      using enum Operation;

    case Zoom:
      zoom(camera, mouseDelta, settings.zoomSpeed);
      break;
    case Rotate:
      if (arcballConfig) {
        arcball(camera, *arcballConfig, mouseDelta, settings.rotateSpeed);
      } else {
        rotate(camera, mouseDelta, settings.rotateSpeed);
      }
      break;
    case Pan:
      pan(camera, mouseDelta, settings.panSpeed);
      break;
    }
    result.dirty = true;
  }
  return result;
}
