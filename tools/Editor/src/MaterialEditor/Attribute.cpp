#include "MaterialEditor/Attribute.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

namespace {

[[nodiscard]] auto getGetterFunctionName(gfx::MaterialDomain materialDomain,
                                         Attribute attribute) {
  switch (attribute) {
    using enum gfx::MaterialDomain;
    using enum Attribute;

  case Position:
    return "getPosition()";
  case Normal:
    return "getNormal()";
  case TexCoord0:
    return materialDomain == Surface ? "getTexCoord0()" : "v_TexCoord";
  case TexCoord1:
    if (materialDomain == Surface) return "getTexCoord1()";
    break;
  case Color:
    return "getColor()";
  }

  assert(false);
  return "";
}

} // namespace

DataType getDataType(Attribute attribute) {
  using enum DataType;

  switch (attribute) {
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

const char *toString(Attribute attribute) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (attribute) {
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

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    Attribute attribute) {
  return ShaderToken{
    .name = getGetterFunctionName(context.materialDomain, attribute),
    .dataType = getDataType(attribute),
  };
}
