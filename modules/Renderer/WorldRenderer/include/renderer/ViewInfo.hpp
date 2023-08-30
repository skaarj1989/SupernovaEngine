#pragma once

#include "PerspectiveCamera.hpp"
#include "Renderable.hpp"

namespace gfx {

struct ViewInfo {
  const PerspectiveCamera &camera;
  std::span<const Renderable *> visibleRenderables;
};

} // namespace gfx
