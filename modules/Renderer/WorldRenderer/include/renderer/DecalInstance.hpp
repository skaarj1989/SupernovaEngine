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

template <> struct entt::type_hash<gfx::DecalInstance> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 976720766;
  }
};
