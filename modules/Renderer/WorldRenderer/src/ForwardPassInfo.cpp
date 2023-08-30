#include "renderer/ForwardPassInfo.hpp"
#include "math/Hash.hpp"

namespace std {

size_t hash<gfx::ForwardPassInfo>::operator()(
  const gfx::ForwardPassInfo &v) const noexcept {
  auto h = hash<gfx::BaseGeometryPassInfo>{}(v);
  hashCombine(h, v.features);
  return h;
}

} // namespace std
