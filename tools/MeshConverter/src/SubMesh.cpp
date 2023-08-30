#include "SubMesh.hpp"

namespace offline {

void SubMesh::addLOD(IndicesList &indexBuffer, const IndicesList &level) {
  LODs.push_back({
    .indexOffset = indexBuffer.size(),
    .numIndices = level.size(),
  });
  indexBuffer.insert(indexBuffer.cend(), level.cbegin(), level.cend());
}

} // namespace offline
