#include "Property.hpp"
#include "RmlUi/Core/PropertyDictionary.h"
#include "glm2Rml.hpp"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

namespace {

void registerVariant(sol::table &lua) {
  // clang-format off
  using VariantType = Variant::Type;
#define MAKE_PAIR(Value) _MAKE_PAIR(VariantType, Value)
  DEFINE_ENUM(VariantType, {
    MAKE_PAIR(NONE),
    MAKE_PAIR(BOOL),
    MAKE_PAIR(BYTE),
    MAKE_PAIR(CHAR),
    MAKE_PAIR(FLOAT),
    MAKE_PAIR(DOUBLE),
    MAKE_PAIR(INT),
    MAKE_PAIR(INT64),
    MAKE_PAIR(UINT),
    MAKE_PAIR(UINT64),
    MAKE_PAIR(STRING),
    MAKE_PAIR(VECTOR2),
    MAKE_PAIR(VECTOR3),
    MAKE_PAIR(VECTOR4),
    MAKE_PAIR(COLOURF),
    MAKE_PAIR(COLOURB),
    MAKE_PAIR(SCRIPTINTERFACE),
    MAKE_PAIR(TRANSFORMPTR),
    MAKE_PAIR(TRANSITIONLIST),
    MAKE_PAIR(ANIMATIONLIST),
    MAKE_PAIR(DECORATORSPTR),
    MAKE_PAIR(FONTEFFECTSPTR),
    MAKE_PAIR(VOIDPTR),
  });
#undef MAKE_PAIR

  DEFINE_USERTYPE(Variant,
    sol::call_constructor,
    sol::factories(
      [](bool b) { return Variant{b}; },
      [](int v) { return Variant{v}; },
      [](float v) { return Variant{v}; },
      [](String s) { return Variant{s}; },
      [](glm::vec2 v) { return Variant{to_Rml(v)}; },
      [](glm::vec3 v) { return Variant{to_Rml(v)}; },
      [](glm::vec4 v) { return Variant{to_Rml(v)}; }
    ),

    sol::meta_function::equal_to, &Variant::operator==,

    "clear", &Variant::Clear,
    "getType", &Variant::GetType,

    BIND_TOSTRING(Variant)
  );
  // clang-format on
}
void registerPropertyDictionary(sol::table &lua) {
  // clang-format off
  DEFINE_USERTYPE(PropertyDictionary,
    sol::call_constructor,
    sol::constructors<PropertyDictionary()>(),

    "setProperty",
      sol::resolve<void(PropertyId, const Property &)>(&PropertyDictionary::SetProperty),
    "removeProperty", &PropertyDictionary::RemoveProperty,
    "getProperty", &PropertyDictionary::GetProperty,
    
    "getNumProperties", &PropertyDictionary::GetNumProperties,

    "import", &PropertyDictionary::Import,
    "merge", &PropertyDictionary::Merge,

    BIND_TOSTRING(PropertyDictionary)
  );
  // clang-format on
}

} // namespace

void registerProperty(sol::table &lua) {
  using PropertyUnit = Property::Unit;
#define MAKE_PAIR(Value) _MAKE_PAIR(PropertyUnit, Value)
  // clang-format off
  DEFINE_ENUM(PropertyUnit, {
    MAKE_PAIR(UNKNOWN),
    MAKE_PAIR(KEYWORD),
    MAKE_PAIR(STRING),
    MAKE_PAIR(NUMBER),
    MAKE_PAIR(PX),
    MAKE_PAIR(DEG),
    MAKE_PAIR(RAD),
    MAKE_PAIR(COLOUR),
    MAKE_PAIR(DP),
    MAKE_PAIR(X),
    MAKE_PAIR(VW),
    MAKE_PAIR(VH),
    MAKE_PAIR(ABSOLUTE_UNIT),
    MAKE_PAIR(EM),
    MAKE_PAIR(PERCENT),
    MAKE_PAIR(REM),
    MAKE_PAIR(RELATIVE_UNIT),
    MAKE_PAIR(INCH),
    MAKE_PAIR(CM),
    MAKE_PAIR(MM),
    MAKE_PAIR(PT),
    MAKE_PAIR(PC),
    MAKE_PAIR(PPI_UNIT),
    MAKE_PAIR(TRANSFORM),
    MAKE_PAIR(TRANSITION),
    MAKE_PAIR(ANIMATION),
    MAKE_PAIR(DECORATOR),
    MAKE_PAIR(FONTEFFECT),
    MAKE_PAIR(RATIO),
    MAKE_PAIR(LENGTH),
    MAKE_PAIR(LENGTH_PERCENT),
    MAKE_PAIR(NUMBER_LENGTH_PERCENT),
    MAKE_PAIR(ABSOLUTE_LENGTH),
    MAKE_PAIR(ANGLE),
  });
  // clang-format on
#undef MAKE_PAIR

  registerVariant(lua);

#define CTOR_2(Type, convert)                                                  \
  [](Type v, Property::Unit unit) { return Property{convert(v), unit}; }
#define CTOR_3(Type, convert)                                                  \
  [](Type v, Property::Unit unit, int specificity) {                           \
    return Property{convert(v), unit, specificity};                            \
  }
#define CTORS(Type, convert) CTOR_2(Type, convert), CTOR_3(Type, convert)

  // clang-format off
  DEFINE_USERTYPE(Property,
    sol::call_constructor,
    sol::factories(
      []{ return Property{}; },
      CTORS(bool, /**/),
      CTORS(int, /**/),
      CTORS(float, /**/),
      CTORS(String, /**/),
      CTORS(glm::vec2, to_Rml),
      CTORS(glm::vec3, to_Rml),
      CTORS(glm::vec4, to_Rml)
    ),

    sol::meta_function::equal_to, &Property::operator==,

    "value", &Property::value,
    "unit", &Property::unit,

    sol::meta_function::to_string, &Property::ToString
  );
  // clang-format on

  registerPropertyDictionary(lua);
}
