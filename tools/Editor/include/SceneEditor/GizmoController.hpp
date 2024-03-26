#pragma once

#include "ImGuizmo.hpp"
#include "entt/entity/fwd.hpp"

class Transform;

namespace gfx {
class PerspectiveCamera;
}

class GizmoController final {
public:
  GizmoController() = delete;

  struct Settings {
    ImGuizmoMode mode{ImGuizmoMode_Local};
    ImGuizmoOperation operation{ImGuizmoOperation_Translate};

    struct Snap {
      using Value = std::pair<float, bool>;
      Value translate{0.1f, false};
      Value rotate{0.01745f /*1deg*/, false};
      Value scale{0.1f, false};
    };
    Snap snap;
  };
  static bool update(const gfx::PerspectiveCamera &, const Settings &,
                     entt::handle);
  static bool update(const gfx::PerspectiveCamera &, const Settings &,
                     Transform &);
};

void showBar(GizmoController::Settings &);
