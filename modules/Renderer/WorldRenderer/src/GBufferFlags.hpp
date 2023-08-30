#pragma once

#include "ScopedEnumFlags.hpp"

namespace gfx {

enum class GBufferFlags {
  Depth = 1 << 0,
  Normal = 1 << 1,
  Emissive = 1 << 2,
  AlbedoSpecular = 1 << 3,
  MetallicRoughnessAO = 1 << 4,
  Misc = 1 << 5,

  All = Depth | Normal | Emissive | AlbedoSpecular | MetallicRoughnessAO | Misc
};
template <> struct has_flags<GBufferFlags> : std::true_type {};

} // namespace gfx
