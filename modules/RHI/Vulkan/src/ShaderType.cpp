#include "rhi/ShaderType.hpp"
#include <cassert>

namespace rhi {

const char *toString(ShaderType shaderType) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (shaderType) {
    using enum ShaderType;

    CASE(Vertex);
    CASE(Geometry);
    CASE(Fragment);
    CASE(Compute);
  }
#undef CASE

  assert(false);
  return "Undefined";
}

ShaderStages getStage(ShaderType shaderType) {
#define CASE(Value)                                                            \
  case ShaderType::Value:                                                      \
    return ShaderStages::Value

  switch (shaderType) {
    CASE(Vertex);
    CASE(Geometry);
    CASE(Fragment);
    CASE(Compute);
  }
#undef CASE

  assert(false);
  return ShaderStages{0};
}

} // namespace rhi
