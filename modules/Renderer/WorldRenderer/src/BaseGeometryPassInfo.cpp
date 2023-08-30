#include "renderer/BaseGeometryPassInfo.hpp"
#include "math/Hash.hpp"

namespace std {

size_t hash<gfx::BaseGeometryPassInfo>::operator()(
  const gfx::BaseGeometryPassInfo &v) const noexcept {
  size_t h{0};
  hashCombine(h, v.depthFormat);
  for (const auto format : v.colorFormats)
    hashCombine(h, format);

  hashCombine(h, v.topology);
  if (v.vertexFormat) hashCombine(h, v.vertexFormat->getHash());
  if (v.material) hashCombine(h, v.material->getHash());
  return h;
}

} // namespace std
