#include "MaterialEditor/Attribute.hpp"
#include <cassert>

DataType getDataType(const Attribute e) {
  using enum DataType;

  switch (e) {
    using enum Attribute;

  case Position:
    return Vec4;
  case TexCoord0:
  case TexCoord1:
    return Vec2;
  case Normal:
    return Vec3;
  case Color:
    return Vec4;
  }

  assert(false);
  return Undefined;
}
const char *toString(const Attribute e) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (e) {
    using enum Attribute;

    CASE(Position);
    CASE(TexCoord0);
    CASE(TexCoord1);
    CASE(Normal);
    CASE(Color);
  }

  assert(false);
  return "Undefined";
}
