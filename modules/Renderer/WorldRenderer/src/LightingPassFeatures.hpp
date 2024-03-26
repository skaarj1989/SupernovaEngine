#pragma once

#include <cstdint>
#include <optional>

class ShaderCodeBuilder;

namespace gfx {

struct LightingPassFeatures {
  std::optional<uint32_t> tileSize{};
  bool ssao{false};
  bool softShadows{false};
  bool globalIllumination{false};
  bool skyLight{false};

  bool irradianceOnly{false};
};

void addLighting(ShaderCodeBuilder &, const LightingPassFeatures &);

} // namespace gfx

namespace std {

template <> struct hash<gfx::LightingPassFeatures> {
  size_t operator()(const gfx::LightingPassFeatures &) const noexcept;
};

} // namespace std
