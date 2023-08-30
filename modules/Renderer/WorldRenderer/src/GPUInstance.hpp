#pragma once

#include <cstdint>

namespace gfx {

struct alignas(16) GPUInstance {
  uint32_t transformId{0};
  uint32_t skinOffset{0};
  uint32_t materialId{0};
  uint32_t flags{0};
};
static_assert(sizeof(GPUInstance) == 16);

} // namespace gfx
