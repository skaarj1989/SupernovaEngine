#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <optional>

namespace gfx {
class PerspectiveCamera;
}

class CameraController final {
public:
  CameraController() = delete;

  struct Settings {
    float rotateSpeed{0.25f};
    float panSpeed{0.05f};
    float zoomSpeed{0.025f};
  };
  struct ArcBallConfig {
    glm::vec3 pivot;
    glm::vec2 viewport;
  };

  struct Result {
    bool wantUse{false};
    bool dirty{false};
  };
  static Result update(gfx::PerspectiveCamera &, const Settings &,
                       const glm::vec2 mouseDelta,
                       const std::optional<ArcBallConfig>);
};
