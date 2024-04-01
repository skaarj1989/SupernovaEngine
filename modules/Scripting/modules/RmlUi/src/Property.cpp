#include "Property.hpp"
#include "RmlUi/Core/PropertyDictionary.h"
#include "glm2Rml.hpp"
#include "Rml2glm.hpp"

#include "Sol2HelperMacros.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wswitch"
#endif

using namespace Rml;

namespace {

void registerVariant(sol::table &lua) {
  // clang-format off
  using VariantType = Variant::Type;
#define MAKE_PAIR(Value) _MAKE_PAIR(VariantType, Value)
  lua.DEFINE_ENUM(VariantType, {
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

  lua.DEFINE_USERTYPE(Variant,
    sol::call_constructor,
    sol::factories(
      [](bool b) { return Variant{b}; },
      [](int v) { return Variant{v}; },
      [](float v) { return Variant{v}; },
      [](const String &s) { return Variant{s}; },
      [](glm::vec2 v) { return Variant{to_Rml(v)}; },
      [](const glm::vec3 &v) { return Variant{to_Rml(v)}; },
      [](const glm::vec4 &v) { return Variant{to_Rml(v)}; }
    ),

    sol::meta_function::equal_to, &Variant::operator==,

    "clear", &Variant::Clear,
    "getType", &Variant::GetType,

    "get", [](const Variant &self, sol::this_state s) {
      switch (self.GetType()) {
        using enum Variant::Type;

        case BOOL: return sol::make_object(s, self.Get<bool>());
        case INT: return sol::make_object(s, self.Get<int>());
        case FLOAT: return sol::make_object(s, self.Get<float>());
        case STRING: return sol::make_object(s, self.Get<String>());
        case VECTOR2: return sol::make_object(s, to_glm(self.Get<Vector2f>()));
        case VECTOR3: return sol::make_object(s, to_glm(self.Get<Vector3f>()));
        case VECTOR4: return sol::make_object(s, to_glm(self.Get<Vector4f>()));
      }
      return sol::object{};
    },

    BIND_TOSTRING(Variant)
  );
  // clang-format on
}
void registerPropertyDictionary(sol::table &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(PropertyDictionary,
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

void registerUnit(sol::table &lua) {
#define MAKE_PAIR(Value) _MAKE_PAIR(Unit, Value)
  // clang-format off
  lua.DEFINE_ENUM(Unit, {
    MAKE_PAIR(UNKNOWN),

    MAKE_PAIR(KEYWORD),
    MAKE_PAIR(STRING),
    MAKE_PAIR(COLOUR),
    MAKE_PAIR(RATIO),

    MAKE_PAIR(NUMBER),
    MAKE_PAIR(PERCENT),
    MAKE_PAIR(PX),

    MAKE_PAIR(DP),
    MAKE_PAIR(VW),
    MAKE_PAIR(VH),
    MAKE_PAIR(X),

    MAKE_PAIR(EM),
    MAKE_PAIR(REM),

    MAKE_PAIR(INCH),
    MAKE_PAIR(CM),
    MAKE_PAIR(MM),
    MAKE_PAIR(PT),
    MAKE_PAIR(PC),
    MAKE_PAIR(PPI_UNIT),

    MAKE_PAIR(DEG),
    MAKE_PAIR(RAD),

    MAKE_PAIR(TRANSFORM),
    MAKE_PAIR(TRANSITION),
    MAKE_PAIR(ANIMATION),
    MAKE_PAIR(DECORATOR),
    MAKE_PAIR(FONTEFFECT),
    MAKE_PAIR(COLORSTOPLIST),
    MAKE_PAIR(BOXSHADOWLIST),

    MAKE_PAIR(LENGTH),
    MAKE_PAIR(LENGTH_PERCENT),
    MAKE_PAIR(NUMBER_LENGTH_PERCENT),
    MAKE_PAIR(DP_SCALABLE_LENGTH),
    MAKE_PAIR(ANGLE),
    MAKE_PAIR(NUMERIC),
  });
  // clang-format on
#undef MAKE_PAIR
}
void registerProperty(sol::table &lua) {
  registerUnit(lua);
  registerVariant(lua);

#define CTOR_2(Type, convert)                                                  \
  [](Type v, Unit unit) { return Property{convert(v), unit}; }
#define CTOR_3(Type, convert)                                                  \
  [](Type v, Unit unit, int specificity) {                                     \
    return Property{convert(v), unit, specificity};                            \
  }
#define CTORS(Type, convert) CTOR_2(Type, convert), CTOR_3(Type, convert)

  // clang-format off
  lua.DEFINE_USERTYPE(Property,
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
