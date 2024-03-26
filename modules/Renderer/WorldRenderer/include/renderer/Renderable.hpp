#pragma once

#include <unordered_map>
#include <span>

namespace gfx {

class Mesh;
struct SubMeshInstance;

class Material;

struct Renderable {
  const Mesh *mesh{nullptr};
  const SubMeshInstance &subMeshInstance;

  uint32_t transformId{UINT_MAX}; // Index to frame's global modelMatrices.
  uint32_t skinOffset{UINT_MAX};  // Offset to the first joint.
  uint32_t materialId{UINT_MAX};
};

const Material *getMaterial(const Renderable &);

void sortByMaterial(std::span<const Renderable *>);

// Key = Buffer size (in bytes).
// Value = Offset (in bytes).
using PropertyGroupOffsets = std::unordered_map<std::size_t, std::size_t>;

} // namespace gfx
