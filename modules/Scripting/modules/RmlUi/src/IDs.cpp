#include "IDs.hpp"
#include "RmlUi/Core/ID.h"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

namespace {

void registerShorthandId(sol::table &lua) {
#define MAKE_PAIR(Value) _MAKE_PAIR(ShorthandId, Value)
  // clang-format off
  lua.DEFINE_ENUM(ShorthandId, {
    MAKE_PAIR(Invalid),
    MAKE_PAIR(Margin),
    MAKE_PAIR(Padding),
    MAKE_PAIR(BorderWidth),
    MAKE_PAIR(BorderColor),
    MAKE_PAIR(BorderTop),
    MAKE_PAIR(BorderRight),
    MAKE_PAIR(BorderBottom),
    MAKE_PAIR(BorderLeft),
    MAKE_PAIR(Border),
    MAKE_PAIR(BorderRadius),
    MAKE_PAIR(Overflow),
    MAKE_PAIR(Background),
    MAKE_PAIR(Font),
    MAKE_PAIR(Gap),
    MAKE_PAIR(PerspectiveOrigin),
    MAKE_PAIR(TransformOrigin),
    MAKE_PAIR(Flex),
    MAKE_PAIR(FlexFlow),

    MAKE_PAIR(NumDefinedIds),
    MAKE_PAIR(FirstCustomId),

    MAKE_PAIR(MaxNumIds),
  });
  // clang-format on
#undef MAKE_PAIR
}
void registerPropertyId(sol::table &lua) {
#define MAKE_PAIR(Value) _MAKE_PAIR(PropertyId, Value)
  // clang-format off
  lua.DEFINE_ENUM(PropertyId, {
    MAKE_PAIR(Invalid),

    MAKE_PAIR(MarginTop),
    MAKE_PAIR(MarginRight),
    MAKE_PAIR(MarginBottom),
    MAKE_PAIR(MarginLeft),
    MAKE_PAIR(PaddingTop),
    MAKE_PAIR(PaddingRight),
    MAKE_PAIR(PaddingBottom),
    MAKE_PAIR(PaddingLeft),
    MAKE_PAIR(BorderTopWidth),
    MAKE_PAIR(BorderRightWidth),
    MAKE_PAIR(BorderBottomWidth),
    MAKE_PAIR(BorderLeftWidth),
    MAKE_PAIR(BorderTopColor),
    MAKE_PAIR(BorderRightColor),
    MAKE_PAIR(BorderBottomColor),
    MAKE_PAIR(BorderLeftColor),
    MAKE_PAIR(BorderTopLeftRadius),
    MAKE_PAIR(BorderTopRightRadius),
    MAKE_PAIR(BorderBottomRightRadius),
    MAKE_PAIR(BorderBottomLeftRadius),
    MAKE_PAIR(Display),
    MAKE_PAIR(Position),
    MAKE_PAIR(Top),
    MAKE_PAIR(Right),
    MAKE_PAIR(Bottom),
    MAKE_PAIR(Left),
    MAKE_PAIR(Float),
    MAKE_PAIR(Clear),
    MAKE_PAIR(BoxSizing),
    MAKE_PAIR(ZIndex),
    MAKE_PAIR(Width),
    MAKE_PAIR(MinWidth),
    MAKE_PAIR(MaxWidth),
    MAKE_PAIR(Height),
    MAKE_PAIR(MinHeight),
    MAKE_PAIR(MaxHeight),
    MAKE_PAIR(LineHeight),
    MAKE_PAIR(VerticalAlign),
    MAKE_PAIR(OverflowX),
    MAKE_PAIR(OverflowY),
    MAKE_PAIR(Clip),
    MAKE_PAIR(Visibility),
    MAKE_PAIR(BackgroundColor),
    MAKE_PAIR(Color),
    MAKE_PAIR(CaretColor),
    MAKE_PAIR(ImageColor),
    MAKE_PAIR(FontFamily),
    MAKE_PAIR(FontStyle),
    MAKE_PAIR(FontWeight),
    MAKE_PAIR(FontSize),
    MAKE_PAIR(TextAlign),
    MAKE_PAIR(TextDecoration),
    MAKE_PAIR(TextTransform),
    MAKE_PAIR(WhiteSpace),
    MAKE_PAIR(WordBreak),
    MAKE_PAIR(RowGap),
    MAKE_PAIR(ColumnGap),
    MAKE_PAIR(Cursor),
    MAKE_PAIR(Drag),
    MAKE_PAIR(TabIndex),
    MAKE_PAIR(ScrollbarMargin),
    MAKE_PAIR(OverscrollBehavior),

    MAKE_PAIR(Perspective),
    MAKE_PAIR(PerspectiveOriginX),
    MAKE_PAIR(PerspectiveOriginY),
    MAKE_PAIR(Transform),
    MAKE_PAIR(TransformOriginX),
    MAKE_PAIR(TransformOriginY),
    MAKE_PAIR(TransformOriginZ),

    MAKE_PAIR(Transition),
    MAKE_PAIR(Animation),

    MAKE_PAIR(Opacity),
    MAKE_PAIR(PointerEvents),
    MAKE_PAIR(Focus),

    MAKE_PAIR(Decorator),
    MAKE_PAIR(FontEffect),

    MAKE_PAIR(FillImage),

    MAKE_PAIR(AlignContent),
    MAKE_PAIR(AlignItems),
    MAKE_PAIR(AlignSelf),
    MAKE_PAIR(FlexBasis),
    MAKE_PAIR(FlexDirection),
    MAKE_PAIR(FlexGrow),
    MAKE_PAIR(FlexShrink),
    MAKE_PAIR(FlexWrap),
    MAKE_PAIR(JustifyContent),

    MAKE_PAIR(NumDefinedIds),
    MAKE_PAIR(FirstCustomId),

    MAKE_PAIR(MaxNumIds),
  });
  // clang-format on
#undef MAKE_PAIR
}
void registerEventId(sol::table &lua) {
#define MAKE_PAIR(Value) _MAKE_PAIR(EventId, Value)
  // clang-format off
  lua.DEFINE_ENUM(EventId, {
    MAKE_PAIR(Invalid),

	  MAKE_PAIR(Mousedown),
	  MAKE_PAIR(Mousescroll),
	  MAKE_PAIR(Mouseover),
	  MAKE_PAIR(Mouseout),
	  MAKE_PAIR(Focus),
	  MAKE_PAIR(Blur),
	  MAKE_PAIR(Keydown),
	  MAKE_PAIR(Keyup),
	  MAKE_PAIR(Textinput),
	  MAKE_PAIR(Mouseup),
	  MAKE_PAIR(Click),
	  MAKE_PAIR(Dblclick),
	  MAKE_PAIR(Load),
	  MAKE_PAIR(Unload),
	  MAKE_PAIR(Show),
	  MAKE_PAIR(Hide),
	  MAKE_PAIR(Mousemove),
	  MAKE_PAIR(Dragmove),
	  MAKE_PAIR(Drag),
	  MAKE_PAIR(Dragstart),
	  MAKE_PAIR(Dragover),
	  MAKE_PAIR(Dragdrop),
	  MAKE_PAIR(Dragout),
	  MAKE_PAIR(Dragend),
	  MAKE_PAIR(Handledrag),
	  MAKE_PAIR(Resize),
	  MAKE_PAIR(Scroll),
	  MAKE_PAIR(Animationend),
	  MAKE_PAIR(Transitionend),

	  MAKE_PAIR(Change),
	  MAKE_PAIR(Submit),
	  MAKE_PAIR(Tabchange),
	  MAKE_PAIR(Columnadd),
	  MAKE_PAIR(Rowadd),
	  MAKE_PAIR(Rowchange),
	  MAKE_PAIR(Rowremove),
	  MAKE_PAIR(Rowupdate),

	  MAKE_PAIR(NumDefinedIds),

	  MAKE_PAIR(FirstCustomId),

	  MAKE_PAIR(MaxNumIds),
  });
  // clang-format on
#undef MAKE_PAIR
}

} // namespace

void registerIDs(sol::table &lua) {
  registerShorthandId(lua);
  registerPropertyId(lua);
  registerEventId(lua);
}
