#include "rhi/CullMode.hpp"
#include <cassert>

namespace rhi {

const char *toString(CullMode cullMode) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (cullMode) {
    using enum CullMode;

    CASE(None);
    CASE(Front);
    CASE(Back);
  }
  assert(false);
  return "Undefined";
}

} // namespace rhi
