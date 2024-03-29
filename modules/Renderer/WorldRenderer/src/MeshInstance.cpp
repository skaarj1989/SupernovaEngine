#include "renderer/MeshInstance.hpp"
#include "Transform.hpp"
#include "spdlog/spdlog.h"

#include <algorithm> // count_if

namespace gfx {

MeshInstance::MeshInstance(std::shared_ptr<Mesh> mesh) : m_prototype{mesh} {
  reset();
}
MeshInstance::MeshInstance(const MeshInstance &other) = default;
MeshInstance::~MeshInstance() = default;

MeshInstance::operator bool() const { return m_prototype != nullptr; }

const Mesh *MeshInstance::operator->() const noexcept {
  return m_prototype.get();
}

const std::shared_ptr<Mesh> &MeshInstance::getPrototype() const {
  return m_prototype;
}

MeshInstance &MeshInstance::show(const bool b) {
  for (auto &sm : each()) {
    sm.visible = b;
  }
  return *this;
}
std::size_t MeshInstance::countVisible() const {
  return std::ranges::count_if(each(),
                               [](const auto &sm) { return sm.visible; });
}

MeshInstance &MeshInstance::setTransform(const Transform &xf) {
  m_modelMatrix = xf.getWorldMatrix();
  _updateAABB(m_modelMatrix);
  return *this;
}
MeshInstance &MeshInstance::setMaterial(const uint32_t index,
                                        std::shared_ptr<Material> material) {
  if (index >= m_subMeshes.size()) {
    SPDLOG_WARN("Invalid material index");
    return *this;
  }
  if (!material || !isSurface(*material)) {
    SPDLOG_WARN("Invalid material");
    return *this;
  }
  m_subMeshes[index].material = MaterialInstance{material};
  return *this;
}

MeshInstance &MeshInstance::setSkinMatrices(Joints skin) {
  m_skinMatrices = std::move(skin);
  return *this;
}

const glm::mat4 &MeshInstance::getModelMatrix() const { return m_modelMatrix; }
const AABB &MeshInstance::getAABB() const { return m_aabb; }

MaterialInstance &MeshInstance::getMaterial(const uint32_t index) {
  assert(index < m_subMeshes.size());
  return m_subMeshes[index].material;
}

bool MeshInstance::hasSkin() const { return !m_skinMatrices.empty(); }
const Joints &MeshInstance::getSkinMatrices() const { return m_skinMatrices; }

MeshInstance &MeshInstance::reset() {
  if (m_prototype) {
    _initialize();
    _updateAABB(m_modelMatrix);
  }
  return *this;
}

//
// (private):
//

void MeshInstance::_initialize() {
  if (!m_prototype) return;

  m_subMeshes.clear();
  const auto &src = m_prototype->getSubMeshes();
  m_subMeshes.reserve(src.size());
  std::ranges::transform(src, std::back_inserter(m_subMeshes),
                         [](const SubMesh &subMesh) {
                           return SubMeshInstance{
                             .prototype = &subMesh,
                             .material = MaterialInstance{subMesh.material},
                             .aabb = subMesh.aabb,
                           };
                         });
}

void MeshInstance::_updateAABB(const glm::mat4 &m) {
  if (!m_prototype) return;

  m_aabb = m_prototype->getAABB().transform(m);
  for (auto &sm : m_subMeshes)
    sm.aabb = sm.prototype->aabb.transform(m);
}

} // namespace gfx
