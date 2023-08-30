#include "MaterialEditor/CameraBlockMember.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <format>

namespace {

[[nodiscard]] auto getMemberName(CameraBlockMember e) {
  switch (e) {
    using enum CameraBlockMember;

  case Projection:
    return "projection";
  case InversedProjection:
    return "inversedProjection";
  case View:
    return "view";
  case InversedView:
    return "inversedView";
  case ViewProjection:
    return "viewProjection";
  case InversedViewProjection:
    return "inversedViewProjection";
  case Resolution:
    return "resolution";
  case Near:
    return "near";
  case Far:
    return "far";
  }

  assert(false);
  return "";
}

} // namespace

DataType getDataType(CameraBlockMember e) {
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
  }

  assert(false);
  return Undefined;
}

const char *toString(CameraBlockMember e) {
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
  }

  assert(false);
  return "Undefined";
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    CameraBlockMember e) {
  return ShaderToken{
    .name = std::format("u_Camera.{}", getMemberName(e)),
    .dataType = getDataType(e),
  };
}
