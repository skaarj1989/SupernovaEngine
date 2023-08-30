#pragma once

#include <cstdint>

namespace gfx {

enum class OutputMode : uint32_t {
  Depth = 0,
  Normal,
  Emissive,
  BaseColor,
  Metallic,
  Roughness,
  AmbientOcclusion,

  SSAO,
  BrightColor,
  Reflections,

  Accum,
  Reveal,

  LightHeatmap,

  HDR,
  FinalImage,
};

} // namespace gfx
