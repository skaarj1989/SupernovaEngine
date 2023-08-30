#pragma once

#include "ScopedEnumFlags.hpp"
#include <string>

namespace gfx {

enum class RenderFeatures : int32_t {
  None = 0,

  LightCulling = 1 << 0,
  SoftShadows = 1 << 1,
  GI = 1 << 2,
  SSAO = 1 << 3,
  SSR = 1 << 4,
  Bloom = 1 << 5,
  FXAA = 1 << 6,
  EyeAdaptation = 1 << 7,
  CustomPostprocess = 1 << 8,

  Default =
    LightCulling | SSAO | Bloom | FXAA | EyeAdaptation | CustomPostprocess,

  All = Default | SoftShadows | GI | SSR,
};
template <> struct has_flags<RenderFeatures> : std::true_type {};

[[nodiscard]] std::string toString(RenderFeatures);

} // namespace gfx
