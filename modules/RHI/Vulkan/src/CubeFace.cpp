#include "rhi/CubeFace.hpp"
#include <cassert>

namespace rhi {

const char *toString(CubeFace cubeFace) {
  switch (cubeFace) {
    using enum CubeFace;

  case PositiveX:
    return "+X";
  case NegativeX:
    return "-X";
  case PositiveY:
    return "+Y";
  case NegativeY:
    return "-Y";
  case PositiveZ:
    return "+Z";
  case NegativeZ:
    return "-Z";
  }
  assert(false);
  return "Undefined";
}

} // namespace rhi
