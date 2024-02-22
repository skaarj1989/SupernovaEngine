#include "TransformPrimitives.hpp"
#include "RmlUi/Core/Transform.h"
#include "RmlUi/Core/Property.h"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

namespace {

void registerTransformPrimitives(sol::table &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(NumericValue,
    sol::call_constructor,
    sol::constructors<
      NumericValue(),
      NumericValue(float, Unit)
    >(),

    "number", &NumericValue::number,
    "unit", &NumericValue::unit,

    BIND_TOSTRING(NumericValue)
  );

  using namespace Transforms;

  lua.DEFINE_USERTYPE(Matrix2D,
    sol::call_constructor,
    sol::constructors<Matrix2D(const NumericValue *)>(),
    BIND_TOSTRING(Matrix2D)
  );
  lua.DEFINE_USERTYPE(Matrix3D,
    sol::call_constructor,
    sol::constructors<Matrix3D(const NumericValue *)>(),
    BIND_TOSTRING(Matrix3D)
  );

#define DEFINE_NUMERIC(Type, ...)                                              \
  lua.DEFINE_USERTYPE(Type,                                                        \
    sol::call_constructor,                                                     \
    sol::constructors<Type(const NumericValue *), __VA_ARGS__>(),              \
    BIND_TOSTRING(Type))

  DEFINE_NUMERIC(TranslateX, TranslateX(float, Unit));
  DEFINE_NUMERIC(TranslateY, TranslateY(float, Unit));
  DEFINE_NUMERIC(TranslateZ, TranslateZ(float, Unit));
  DEFINE_NUMERIC(Translate2D, Translate2D(float, float, Unit));
  DEFINE_NUMERIC(Translate3D, Translate3D(float, float, float, Unit));

  DEFINE_NUMERIC(ScaleX, ScaleX(float));
  DEFINE_NUMERIC(ScaleY, ScaleY(float));
  DEFINE_NUMERIC(ScaleZ, ScaleZ(float));
  DEFINE_NUMERIC(Scale2D, Scale2D(float), Scale2D(float, float));
  DEFINE_NUMERIC(Scale3D, Scale3D(float), Scale3D(float, float, float));

  DEFINE_NUMERIC(RotateX, RotateX(float, Unit));
  DEFINE_NUMERIC(RotateY, RotateY(float, Unit));
  DEFINE_NUMERIC(RotateZ, RotateZ(float, Unit));
  DEFINE_NUMERIC(Rotate2D, Rotate2D(float, Unit));
  DEFINE_NUMERIC(Rotate3D, Rotate3D(float, float, float, float, Unit));
  
  DEFINE_NUMERIC(SkewX, SkewX(float, Unit));
  DEFINE_NUMERIC(SkewY, SkewY(float, Unit));
  DEFINE_NUMERIC(Skew2D, Skew2D(float, float, Unit));

#undef DEFINE_NUMERIC

  lua.DEFINE_USERTYPE(Perspective,
    sol::call_constructor,
    sol::constructors<Perspective(const NumericValue *)>(),
    BIND_TOSTRING(Perspective)
  );
  // clang-format on
}

} // namespace

void registerTransform(sol::table &lua) {
  auto m = lua["Transforms"].get_or_create<sol::table>();
  registerTransformPrimitives(m);

  // clang-format off
  lua.DEFINE_USERTYPE(TransformPrimitive,
    sol::call_constructor,
    sol::constructors<
      TransformPrimitive(Transforms::Matrix2D),
      TransformPrimitive(Transforms::Matrix3D),
      TransformPrimitive(Transforms::TranslateX),
      TransformPrimitive(Transforms::TranslateY),
      TransformPrimitive(Transforms::TranslateZ),
      TransformPrimitive(Transforms::Translate2D),
      TransformPrimitive(Transforms::Translate3D),
      TransformPrimitive(Transforms::ScaleX),
      TransformPrimitive(Transforms::ScaleY),
      TransformPrimitive(Transforms::ScaleZ),
      TransformPrimitive(Transforms::Scale2D),
      TransformPrimitive(Transforms::Scale3D),
      TransformPrimitive(Transforms::SkewX),
      TransformPrimitive(Transforms::SkewY),
      TransformPrimitive(Transforms::Skew2D),
      TransformPrimitive(Transforms::Perspective),
      TransformPrimitive(Transforms::DecomposedMatrix4)
    >(),

    _BIND(TransformPrimitive, type),

    BIND_TOSTRING(TransformPrimitive)
  );

#define MAKE_PAIR(Value) _MAKE_PAIR(TransformPrimitive::Type, Value)
  lua DEFINE_NESTED_ENUM(TransformPrimitive, Type, {
    MAKE_PAIR(MATRIX2D), MAKE_PAIR(MATRIX3D),
    MAKE_PAIR(TRANSLATEX), MAKE_PAIR(TRANSLATEY), MAKE_PAIR(TRANSLATEZ),
    MAKE_PAIR(TRANSLATE2D), MAKE_PAIR(TRANSLATE3D), MAKE_PAIR(TRANSLATE3D),
    MAKE_PAIR(SCALEX), MAKE_PAIR(SCALEY), MAKE_PAIR(SCALEZ),
    MAKE_PAIR(SCALE2D), MAKE_PAIR(SCALE3D),
    MAKE_PAIR(ROTATEX), MAKE_PAIR(ROTATEY), MAKE_PAIR(ROTATEZ),
    MAKE_PAIR(ROTATE2D), MAKE_PAIR(ROTATE3D),
    MAKE_PAIR(SKEWX), MAKE_PAIR(SKEWY), MAKE_PAIR(SCALE2D),
    MAKE_PAIR(PERSPECTIVE), MAKE_PAIR(DECOMPOSEDMATRIX4),
  });
#undef MAKE_PAIR

  lua.DEFINE_USERTYPE(Transform, 
    sol::call_constructor,
    sol::constructors<
      Transform(),
      Transform(Transform::PrimitiveList)
    >(),

    "makeProperty", Transform::MakeProperty,

    "clearPrimitives", &Transform::ClearPrimitives,
    "addPrimitive", &Transform::AddPrimitive,
    "getNumPrimitives", &Transform::GetNumPrimitives,
    "getPrimitive", &Transform::GetPrimitive,
    "getPrimitives", sol::overload(
      sol::resolve<Transform::PrimitiveList &()>(&Transform::GetPrimitives),
      sol::resolve<const Transform::PrimitiveList &() const>(&Transform::GetPrimitives)
    ),

    BIND_TOSTRING(Transform)
  );
  // clang-format on
}
