#pragma once

#include "math/AABB.hpp"
#include "SceneView.hpp"

namespace gfx {

struct Light;
class MeshInstance;
class DecalInstance;

struct WorldView {
  AABB aabb;
  std::span<const Light *> lights;
  std::span<const MeshInstance *> meshes;
  std::span<const DecalInstance *> decals;
  std::span<const SceneView> sceneViews;
};

} // namespace gfx
