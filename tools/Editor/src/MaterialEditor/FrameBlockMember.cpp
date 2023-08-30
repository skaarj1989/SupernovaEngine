#include "MaterialEditor/FrameBlockMember.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <format>

namespace {

[[nodiscard]] auto getMemberName(FrameBlockMember e) {
  switch (e) {
    using enum FrameBlockMember;

  case Time:
    return "time";
  case DeltaTime:
    return "deltaTime";
  }

  assert(false);
  return "";
}

} // namespace

DataType getDataType(FrameBlockMember e) {
  using enum DataType;
  switch (e) {
    using enum FrameBlockMember;

  case Time:
  case DeltaTime:
    return Float;
  }

  assert(false);
  return Undefined;
}

const char *toString(FrameBlockMember e) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (e) {
    using enum FrameBlockMember;

    CASE(Time);
    CASE(DeltaTime);
  }

  assert(false);
  return "Undefined";
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    FrameBlockMember e) {
  return ShaderToken{
    .name = std::format("u_Frame.{}", getMemberName(e)),
    .dataType = getDataType(e),
  };
}
