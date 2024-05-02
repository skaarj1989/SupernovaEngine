#pragma once

#include "MaterialInstance.hpp"

class DebugDraw;

namespace rhi {
class Texture;
class Buffer;
} // namespace rhi

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

  // Used to store `userData` of Mesh/DecalInstance (if provided).
  // The buffer size should equal to `target` extent, with a stride of uint.
  // (width * height * sizeof(uint32_t)).
  // To get an ID at a given coordinate [x, y], use: `x + width * y`.
  rhi::Buffer *userData{nullptr};
};

} // namespace gfx
