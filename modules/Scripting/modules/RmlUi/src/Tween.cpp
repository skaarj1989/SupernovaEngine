#include "Tween.hpp"
#include "RmlUi/Core/Tween.h"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

void registerTween(sol::table &lua) {
  // clang-format off
  using TweenType = Tween::Type;
#define MAKE_PAIR(Value) _MAKE_PAIR(TweenType, Value)
  DEFINE_ENUM(TweenType, {
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

  using TweenDirection = Tween::Direction;
#define MAKE_PAIR(Value) _MAKE_PAIR(TweenDirection, Value)
  DEFINE_ENUM(TweenDirection, {
    MAKE_PAIR(In),
    MAKE_PAIR(Out),
    MAKE_PAIR(InOut),
  });
#undef MAKE_PAIR

  DEFINE_USERTYPE(Tween,
    sol::call_constructor,
    sol::constructors<
      Tween(),
      Tween(TweenType),
      Tween(TweenType, TweenDirection),
      Tween(TweenType, TweenType)
    >(),

    "reverse", &Tween::reverse,

    sol::meta_function::to_string, &Tween::to_string
  );
  // clang-format on
}
