#include "renderer/Renderable.hpp"
#include "renderer/MeshInstance.hpp"

namespace gfx {

const Material *getMaterial(const Renderable &renderable) {
  return renderable.subMeshInstance.material.getPrototype().get();
}

void sortByMaterial(std::span<const Renderable *> renderables) {
  std::ranges::sort(renderables, [](const Renderable *a, const Renderable *b) {
    return getMaterial(*a)->getHash() < getMaterial(*b)->getHash();
  });
}

} // namespace gfx
