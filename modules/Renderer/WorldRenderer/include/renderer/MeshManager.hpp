#pragma once

#include "MeshLoader.hpp"
#include "MaterialManager.hpp"

namespace gfx {

using MeshCache = entt::resource_cache<MeshResource, MeshLoader>;

class MeshManager final : public MeshCache {
public:
  MeshManager(rhi::RenderDevice &, MaterialManager &);
  ~MeshManager() = default;

  [[nodiscard]] bool isBuiltIn(const std::filesystem::path &) const;
  [[nodiscard]] bool isBuiltIn(const uint32_t id) const;

  [[nodiscard]] MeshResourceHandle load(const std::filesystem::path &);
  void import(const std::string_view name, Mesh &&);

  void clear();

  struct BasicShapes {
    BasicShapes() = delete;

    static const entt::hashed_string Plane;
    static const entt::hashed_string SubdividedPlane;
    static const entt::hashed_string Cube;
    static const entt::hashed_string Sphere;
  };

private:
  rhi::RenderDevice &m_renderDevice;
  MaterialManager &m_materialManager;
};

} // namespace gfx
