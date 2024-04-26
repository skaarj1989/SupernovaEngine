#pragma once

#include <cstdint>

namespace gfx {

struct GPUInstance {
  uint32_t transformId{0};
  uint32_t skinOffset{0};
  uint32_t materialId{0};
  uint32_t flags{0};
  uint32_t userData{0};
};

} // namespace gfx
