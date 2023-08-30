#pragma once

#include "ScopedEnumFlags.hpp"

namespace gfx {

enum class PipelineStage {
  Transfer = 1 << 0,
  VertexShader = 1 << 1,
  GeometryShader = 1 << 2,
  FragmentShader = 1 << 3,
  ComputeShader = 1 << 4,
};
template <> struct has_flags<PipelineStage> : std::true_type {};

} // namespace gfx
