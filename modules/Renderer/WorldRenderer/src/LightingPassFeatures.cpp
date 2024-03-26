#include "LightingPassFeatures.hpp"
#include "math/Hash.hpp"
#include "ShaderCodeBuilder.hpp"
#include <cassert>

namespace gfx {

void addLighting(ShaderCodeBuilder &builder,
                 const LightingPassFeatures &features) {
  builder.addDefine("TILED_LIGHTING", uint32_t(features.tileSize.has_value()));
  if (features.tileSize) {
    assert(*features.tileSize > 0);
    builder.addDefine("TILE_SIZE", *features.tileSize);
  }
  builder.addDefine("SOFT_SHADOWS", uint32_t(features.softShadows))
    .addDefine("HAS_GLOBAL_ILLUMINATION", uint32_t(features.globalIllumination))
    .addDefine("HAS_SKYLIGHT", uint32_t(features.skyLight))
    .addDefine("IRRADIANCE_ONLY", uint32_t(features.irradianceOnly));

  if (features.ssao) builder.addDefine("HAS_SSAO", 1);
}

} // namespace gfx

namespace std {

size_t hash<gfx::LightingPassFeatures>::operator()(
  const gfx::LightingPassFeatures &v) const noexcept {
  size_t h{0};
  hashCombine(h, v.tileSize.value_or(0u), v.ssao, v.softShadows,
              v.globalIllumination, v.skyLight, v.irradianceOnly);
  return h;
}

} // namespace std
