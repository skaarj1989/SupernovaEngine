#pragma once

#include "Mesh.hpp"

namespace gfx {

class MeshResource final : public Resource, public Mesh {
public:
  MeshResource() = default;
  explicit MeshResource(Mesh &&, const std::filesystem::path &);
  MeshResource(const MeshResource &) = delete;
  MeshResource(MeshResource &&) noexcept = default;

  MeshResource &operator=(const MeshResource &) = delete;
  MeshResource &operator=(MeshResource &&) noexcept = default;
};

} // namespace gfx
