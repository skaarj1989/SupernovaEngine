#include "Tween.hpp"
#include "RmlUi/Core/Tween.h"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

void registerTween(sol::table &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(Tween,
    sol::call_constructor,
    sol::constructors<
      Tween(),
      Tween(Tween::Type),
      Tween(Tween::Type, Tween::Direction),
      Tween(Tween::Type, Tween::Type)
    >(),

    _BIND(Tween, reverse),

    sol::meta_function::to_string, &Tween::to_string
  );

#define MAKE_PAIR(Value) _MAKE_PAIR(Tween::Type, Value)
  lua DEFINE_NESTED_ENUM(Tween, Type, {
    MAKE_PAIR(None),
    MAKE_PAIR(Back),
    MAKE_PAIR(Bounce),
    MAKE_PAIR(Circular),
    MAKE_PAIR(Cubic),
    MAKE_PAIR(Elastic),
    MAKE_PAIR(Exponential),
    MAKE_PAIR(Linear),
    MAKE_PAIR(Quadratic),
    MAKE_PAIR(Quartic),
    MAKE_PAIR(Quintic),
    MAKE_PAIR(Sine),
    MAKE_PAIR(Callback),
    MAKE_PAIR(Count),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Tween::Direction, Value)
  lua DEFINE_NESTED_ENUM(Tween, Direction, {
    MAKE_PAIR(In),
    MAKE_PAIR(Out),
    MAKE_PAIR(InOut),
  });
#undef MAKE_PAIR
  // clang-format on
}
