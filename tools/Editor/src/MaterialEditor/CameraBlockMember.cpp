#include "MaterialEditor/CameraBlockMember.hpp"
#include <cassert>

DataType getDataType(const CameraBlockMember e) {
  using enum DataType;
  switch (e) {
    using enum CameraBlockMember;

  case Projection:
  case InversedProjection:
  case View:
  case InversedView:
  case ViewProjection:
  case InversedViewProjection:
    return Mat4;
  case Resolution:
    return UVec2;
  case Near:
  case Far:
    return Float;

  default:
    assert(false);
    return Undefined;
  }
}
const char *toString(const CameraBlockMember e) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (e) {
    using enum CameraBlockMember;

    CASE(Projection);
    CASE(InversedProjection);
    CASE(View);
    CASE(InversedView);
    CASE(ViewProjection);
    CASE(InversedViewProjection);
    CASE(Resolution);
    CASE(Near);
    CASE(Far);

  default:
    assert(false);
    return "Undefined";
  }
}
