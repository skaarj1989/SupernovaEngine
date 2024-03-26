#include "rhi/VertexAttributes.hpp"
#include <cassert>

namespace rhi {

uint32_t getSize(const VertexAttribute::Type type) {
  switch (type) {
    using enum VertexAttribute::Type;
  case Float:
    return sizeof(float);
  case Float2:
    return sizeof(float) * 2;
  case Float3:
    return sizeof(float) * 3;
  case Float4:
    return sizeof(float) * 4;

  case Int4:
    return sizeof(int32_t) * 4;

  case UByte4_Norm:
    return sizeof(uint8_t) * 4;
  }

  assert(false);
  return 0;
}

} // namespace rhi
