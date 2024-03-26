#pragma once

#include <span>

namespace gfx {

class PerspectiveCamera;
struct Renderable;

struct ViewInfo {
  const PerspectiveCamera &camera;
  std::span<const Renderable *> visibleRenderables;
};

} // namespace gfx
