#include "rhi/PrimitiveTopology.hpp"
#include <cassert>

namespace rhi {

const char *toString(const PrimitiveTopology primitiveTopology) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (primitiveTopology) {
    using enum PrimitiveTopology;

    CASE(PointList);
    CASE(LineList);
    CASE(TriangleList);
    CASE(TriangleStrip);
    CASE(TriangleFan);
  }
  assert(false);
  return "Undefined";
}

} // namespace rhi
