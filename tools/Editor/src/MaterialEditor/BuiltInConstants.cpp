#include "MaterialEditor/BuiltInConstants.hpp"
#include <cassert>

#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

DataType getDataType(const BuiltInConstant e) {
  using enum DataType;
  switch (e) {
    using enum BuiltInConstant;

  case CameraPosition:
    return Vec3;
  case ScreenTexelSize:
    return Vec2;
  case AspectRatio:
    return Float;

  case ModelMatrix:
    return Mat4;
  case FragPosWorldSpace:
  case FragPosViewSpace:
    return Vec3;

  case ViewDir:
    return Vec3;

  default:
    assert(false);
    return Undefined;
  }
}
const char *toString(const BuiltInConstant e) {
  switch (e) {
    using enum BuiltInConstant;

    CASE(CameraPosition);
    CASE(ScreenTexelSize);
    CASE(AspectRatio);

    CASE(ModelMatrix);
    CASE(FragPosWorldSpace);
    CASE(FragPosViewSpace);

    CASE(ViewDir);

  default:
    assert(false);
    return "Undefined";
  }
}

DataType getDataType(const BuiltInSampler) { return DataType::Sampler2D; }
const char *toString(const BuiltInSampler e) {
  switch (e) {
    using enum BuiltInSampler;

    CASE(SceneDepth);
    CASE(SceneColor);

  default:
    assert(false);
    return "Undefined";
  }
}
