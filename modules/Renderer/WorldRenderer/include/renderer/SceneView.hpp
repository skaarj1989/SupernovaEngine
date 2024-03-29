#pragma once

#include "MaterialInstance.hpp"

class DebugDraw;

namespace rhi {
class Texture;
}

namespace gfx {
class PerspectiveCamera;
struct RenderSettings;
struct SkyLight;

struct SceneView {
  const std::string name;

  rhi::Texture &target;
  const PerspectiveCamera &camera;
  const RenderSettings &renderSettings;
  SkyLight *skyLight{nullptr};
  std::span<const MaterialInstance> postProcessEffects;
  DebugDraw *debugDraw{nullptr};
};

} // namespace gfx
