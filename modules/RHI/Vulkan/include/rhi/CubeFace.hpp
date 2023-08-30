#pragma once

namespace rhi {

enum class CubeFace {
  PositiveX = 0,
  NegativeX,
  PositiveY,
  NegativeY,
  PositiveZ,
  NegativeZ,
};

[[nodiscard]] const char *toString(CubeFace);

} // namespace rhi
