#pragma once

#include "MeshManager.hpp"
#include "MaterialInstance.hpp"
#include "entt/core/type_info.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class Transform;

namespace gfx {

struct SubMeshInstance {
  const SubMesh *prototype{nullptr};
  MaterialInstance material;
  bool visible{true};
  bool debug{false};
  AABB aabb; // World-space.

  template <class Archive> void serialize(Archive &archive) {
    archive(visible, material);
  }
};

class MeshInstance {
public:
  explicit MeshInstance(std::shared_ptr<Mesh> = {});
  MeshInstance(const MeshInstance &);
  MeshInstance(MeshInstance &&) noexcept = default;
  virtual ~MeshInstance();

  MeshInstance &operator=(const MeshInstance &) = delete;
  MeshInstance &operator=(MeshInstance &&) noexcept = default;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] const Mesh *operator->() const noexcept;

  [[nodiscard]] const std::shared_ptr<Mesh> &getPrototype() const;

  // ---

  MeshInstance &show(const bool);
  [[nodiscard]] std::size_t countVisible() const;

  virtual MeshInstance &setTransform(const Transform &);
  MeshInstance &setMaterial(const int32_t index, std::shared_ptr<Material>);

  MeshInstance &setSkinMatrices(Joints);

  // ---

  [[nodiscard]] const glm::mat4 &getModelMatrix() const;
  [[nodiscard]] const AABB &getAABB() const;

  [[nodiscard]] MaterialInstance &getMaterial(const uint32_t index);

  [[nodiscard]] bool hasSkin() const;
  [[nodiscard]] const Joints &getSkinMatrices() const;

  MeshInstance &reset();

  // ---

  const auto &each() const { return m_subMeshes; }
  auto &each() { return m_subMeshes; }

  // -- Serialization:

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(m_prototype), m_subMeshes);
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (!path) return;

    m_prototype = loadResource<MeshManager>(*path);
    _initialize();

    archive(m_subMeshes);
  }

private:
  void _initialize();

protected:
  void _updateAABB(const glm::mat4 &);

protected:
  glm::mat4 m_modelMatrix{1.0f};
  AABB m_aabb{}; // World-space.

private:
  std::shared_ptr<Mesh> m_prototype;
  std::vector<SubMeshInstance> m_subMeshes;
  Joints m_skinMatrices;
};

static_assert(std::is_copy_constructible_v<MeshInstance>);

} // namespace gfx

template <> struct entt::type_hash<gfx::MeshInstance> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3477267598;
  }
};
