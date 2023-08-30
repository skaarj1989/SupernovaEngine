#pragma once

#include "MeshInstance.hpp"

namespace gfx {

class DecalInstance final : public MeshInstance {
public:
  explicit DecalInstance(std::shared_ptr<Mesh> = {});
  DecalInstance(const DecalInstance &);
  DecalInstance(DecalInstance &&) noexcept = default;

  DecalInstance &operator=(const DecalInstance &) = delete;
  DecalInstance &operator=(DecalInstance &&) noexcept = default;

  DecalInstance &setTransform(const Transform &) override;
};

static_assert(std::is_copy_constructible_v<DecalInstance>);

} // namespace gfx
