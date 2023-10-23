#pragma once

#include "ScopedEnumFlags.hpp"

namespace gfx {

enum class PipelineGroups {
  SurfaceMaterial = 1 << 0,
  PostProcessMaterial = 1 << 1,
  AllMaterials = SurfaceMaterial | PostProcessMaterial,

  BuiltIn = 1 << 2,

  All = AllMaterials | BuiltIn,
};

} // namespace gfx

template <> struct has_flags<gfx::PipelineGroups> : std::true_type {};
