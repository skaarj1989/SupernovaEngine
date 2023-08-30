#pragma once

#include "Material.hpp"

namespace gfx {

class MaterialResource final : public Resource, public Material {
public:
  MaterialResource() = default;
  MaterialResource(Material &&, const std::filesystem::path &);
  MaterialResource(const MaterialResource &) = delete;
  MaterialResource(MaterialResource &&) noexcept = default;

  MaterialResource &operator=(const MaterialResource &) = delete;
  MaterialResource &operator=(MaterialResource &&) noexcept = default;
};

} // namespace gfx
