#include "MaterialEditor/BuiltInConstants.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

namespace {

[[nodiscard]] auto getGetterFunctionName(BuiltInConstant e) {
  switch (e) {
    using enum BuiltInConstant;

  case CameraPosition:
    return "getCameraPosition()";
  case ScreenTexelSize:
    return "getScreenTexelSize()";
  case AspectRatio:
    return "getAspectRatio()";

  case ModelMatrix:
    return "getModelMatrix()";
  case FragPosWorldSpace:
    return "fragPos.worldSpace";
  case FragPosViewSpace:
    return "fragPos.viewSpace";

  case ViewDir:
    return "getViewDir()";
  }

  assert(false);
  return "";
}

[[nodiscard]] auto getBuiltInSamplerName(BuiltInSampler e) {
  switch (e) {
    using enum BuiltInSampler;

  case SceneDepth:
    return "t_SceneDepth";
  case SceneColor:
    return "t_SceneColor";
  }

  assert(false);
  return "Undefined";
}

} // namespace

DataType getDataType(BuiltInConstant e) {
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
  }

  assert(false);
  return Undefined;
}

const char *toString(BuiltInConstant e) {
  switch (e) {
    using enum BuiltInConstant;

    CASE(CameraPosition);
    CASE(ScreenTexelSize);
    CASE(AspectRatio);

    CASE(ModelMatrix);
    CASE(FragPosWorldSpace);
    CASE(FragPosViewSpace);

    CASE(ViewDir);
  }

  assert(false);
  return "Undefined";
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    BuiltInConstant e) {
  return ShaderToken{
    .name = getGetterFunctionName(e),
    .dataType = getDataType(e),
  };
}

DataType getDataType(BuiltInSampler) { return DataType::Sampler2D; }

const char *toString(BuiltInSampler e) {
  switch (e) {
    using enum BuiltInSampler;

    CASE(SceneDepth);
    CASE(SceneColor);
  }

  assert(false);
  return "Undefined";
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    BuiltInSampler e) {
  return ShaderToken{
    .name = getBuiltInSamplerName(e),
    .dataType = DataType::Sampler2D,
  };
}
