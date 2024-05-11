#include "FrameBlockMember.hpp"
#include <cassert>

DataType getDataType(const FrameBlockMember e) {
  using enum DataType;
  switch (e) {
    using enum FrameBlockMember;

  case Time:
  case DeltaTime:
    return Float;

  default:
    assert(false);
    return Undefined;
  }
}
const char *toString(const FrameBlockMember e) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (e) {
    using enum FrameBlockMember;

    CASE(Time);
    CASE(DeltaTime);

  default:
    assert(false);
    return "Undefined";
  }
}
