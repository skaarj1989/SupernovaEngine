#include "renderer/DecalInstance.hpp"

namespace gfx {

DecalInstance::DecalInstance(std::shared_ptr<Mesh> mesh) : MeshInstance{mesh} {}
DecalInstance::DecalInstance(const DecalInstance &other) = default;

DecalInstance &DecalInstance::setTransform(const Transform &xf) {
  static const auto kProjection =
    glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
  const auto worldMatrix = xf.getWorldMatrix();
  m_modelMatrix = glm::inverse(kProjection * glm::inverse(worldMatrix));
  _updateAABB(worldMatrix);

  return *this;
}

} // namespace gfx
