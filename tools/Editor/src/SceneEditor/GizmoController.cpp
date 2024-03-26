#include "SceneEditor/GizmoController.hpp"
#include "PerspectiveCamera.hpp"
#include "Transform.hpp"
#include "entt/entity/registry.hpp"
#include "entt/entity/handle.hpp"
#include "IconsFontAwesome6.h"
#include <optional>

namespace {

[[nodiscard]] bool showGizmo(const GizmoController::Settings &settings,
                             Transform &xf) {
  auto worldMatrix = xf.getWorldMatrix();
  if (ImGuizmo::Begin(settings.mode, worldMatrix)) {
#define GET_SNAP(Key)                                                          \
  settings.snap.Key.second ? &settings.snap.Key.first : nullptr

    switch (settings.operation) {
    case ImGuizmoOperation_Translate:
      ImGuizmo::Translate(GET_SNAP(translate));
      break;
    case ImGuizmoOperation_Rotate:
      ImGuizmo::Rotate(GET_SNAP(rotate));
      break;
    case ImGuizmoOperation_Scale:
      ImGuizmo::Scale(GET_SNAP(scale));
      break;
    case ImGuizmoOperation_BoundsScale:
      constexpr float kBounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
      ImGuizmo::BoundsScale(kBounds, GET_SNAP(scale));
      break;
    }
#undef GET_SNAP
  }
  static glm::mat4 deltaMatrix{1.0f};
  if (ImGuizmo::End(&deltaMatrix)) {
    switch (settings.operation) {
    case ImGuizmoOperation_Translate:
    case ImGuizmoOperation_BoundsScale: {
      std::optional<glm::mat4> invWorld;
      if (const auto *p = xf.getParent(); p) {
        invWorld = glm::inverse(p->getWorldMatrix());
      }
      xf.load(invWorld ? *invWorld * worldMatrix : worldMatrix);
    } break;
    case ImGuizmoOperation_Rotate:
      xf.rotate(Transform{deltaMatrix}.getOrientation());
      break;
    case ImGuizmoOperation_Scale:
      xf.scale(Transform{deltaMatrix}.getScale());
      break;
    }
    return true;
  }
  return false;
}

} // namespace

bool GizmoController::update(const gfx::PerspectiveCamera &camera,
                             const Settings &settings, entt::handle h) {
  assert(h);
  if (auto *xf = h.try_get<Transform>(); xf) {
    return update(camera, settings, *xf);
  }
  return false;
}
bool GizmoController::update(const gfx::PerspectiveCamera &camera,
                             const Settings &settings, Transform &xf) {
  if (settings.operation == ImGuizmoOperation_None) return false;
  ImGuizmo::SetCamera(camera.getView(), camera.getProjection());
  return showGizmo(settings, xf);
}

namespace {

template <typename Func>
void inspect(const char *label, GizmoController::Settings::Snap::Value &p,
             Func callback) {
  ImGui::PushID(&p);
  ImGui::Checkbox(IM_UNIQUE_ID, &p.second);
  ImGui::SameLine();
  callback(label, p.first);
  ImGui::PopID();
}
void inspect(const char *label, GizmoController::Settings::Snap::Value &p) {
  inspect(label, p, [](const char *label_, auto &v) {
    ImGui::DragFloat(label_, &v, 0.1f);
  });
}

void inspect(GizmoController::Settings::Snap &snap) {
  if (ImGui::BeginMenu(ICON_FA_MAGNET)) {
    ImGui::PushItemWidth(80.0f);
    inspect("Transform", snap.translate);
    inspect("Rotate", snap.rotate, [](const char *label, auto &v) {
      ImGui::SliderAngle(label, &v, 0.1f, 360.0f, "%.1f deg");
    });
    inspect("Scale", snap.scale);
    ImGui::PopItemWidth();
    ImGui::EndMenu();
  }
}

} // namespace

void showBar(GizmoController::Settings &settings) {
  const auto isLocalOnly = [&] {
    return settings.operation == ImGuizmoOperation_Scale ||
           settings.operation == ImGuizmoOperation_BoundsScale;
  };
  if (ImGui::MenuItem(settings.mode == ImGuizmoMode_Local ? "Local" : "World",
                      nullptr, nullptr, !isLocalOnly())) {
    settings.mode = settings.mode == ImGuizmoMode_Local ? ImGuizmoMode_World
                                                        : ImGuizmoMode_Local;
  }
  ImGui::Separator();

  const auto toggle = [&settings](const int32_t op) {
    settings.operation = settings.operation == op ? ImGuizmoOperation_None : op;
  };
  if (auto b = settings.operation == ImGuizmoOperation_Translate;
      ImGui::MenuItem("Translate", nullptr, &b)) {
    toggle(ImGuizmoOperation_Translate);
  }
  if (auto b = settings.operation == ImGuizmoOperation_Rotate;
      ImGui::MenuItem("Rotate", nullptr, &b)) {
    toggle(ImGuizmoOperation_Rotate);
  }
  if (auto b = settings.operation == ImGuizmoOperation_Scale;
      ImGui::MenuItem("Scale", nullptr, &b)) {
    toggle(ImGuizmoOperation_Scale);
    if (isLocalOnly()) settings.mode = ImGuizmoMode_Local;
  }
  if (auto b = settings.operation == ImGuizmoOperation_BoundsScale;
      ImGui::MenuItem("Bounds", nullptr, &b)) {
    toggle(ImGuizmoOperation_BoundsScale);
    if (isLocalOnly()) settings.mode = ImGuizmoMode_Local;
  }
  inspect(settings.snap);
}
