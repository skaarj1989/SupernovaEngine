#pragma once

namespace gfx {

enum class Tonemap : uint32_t {
  Clamp = 0,
  ACES,
  Filmic,
  Reinhard,
  Uncharted,
};

} // namespace gfx
