#pragma once

#include "math/AABB.hpp"
#include "Material.hpp"
#include <optional>

using IndicesList = std::vector<uint32_t>;

namespace offline {

struct SubMesh {
  void addLOD(IndicesList &indexBuffer, const IndicesList &level);

  std::string name;

  std::size_t vertexOffset{0};
  std::size_t numVertices{0};
  struct LOD {
    std::size_t indexOffset{0};
    std::size_t numIndices{0};
  };
  std::vector<LOD> LODs;
  AABB aabb;

  std::optional<offline::Material> material;
};

} // namespace offline
