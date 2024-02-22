#include "Style.hpp"
#include "RmlUi/Core/Box.h"
#include "RmlUi/Core/StyleTypes.h"
#include "RmlUi/Core/StyleSheet.h"
#include "RmlUi/Core/StyleSheetSpecification.h"
#include "glm2Rml.hpp"
#include "Rml2glm.hpp"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

namespace {

void registerStyleTypes(sol::table &lua) {
  using namespace Style;

  // clang-format off
#define MAKE_PAIR(Value) _MAKE_PAIR(Display, Value)
  lua.DEFINE_ENUM(Display, {
    MAKE_PAIR(None),
    MAKE_PAIR(Block),
    MAKE_PAIR(Inline),
    MAKE_PAIR(InlineBlock),
    MAKE_PAIR(Flex),
    MAKE_PAIR(Table),
    MAKE_PAIR(TableRow),
    MAKE_PAIR(TableRowGroup),
    MAKE_PAIR(TableColumn),
    MAKE_PAIR(TableColumnGroup),
    MAKE_PAIR(TableCell),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Position, Value)
  lua.DEFINE_ENUM(Position, {
    MAKE_PAIR(Static),
    MAKE_PAIR(Relative),
    MAKE_PAIR(Absolute),
    MAKE_PAIR(Fixed),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Float, Value)
  lua.DEFINE_ENUM(Float, {
    MAKE_PAIR(None),
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Clear, Value)
  lua.DEFINE_ENUM(Clear, {
    MAKE_PAIR(None),
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
    MAKE_PAIR(Both),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(BoxSizing, Value)
  lua.DEFINE_ENUM(BoxSizing, {
    MAKE_PAIR(ContentBox),
    MAKE_PAIR(BorderBox),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Overflow, Value)
  lua.DEFINE_ENUM(Overflow, {
    MAKE_PAIR(Visible),
    MAKE_PAIR(Hidden),
    MAKE_PAIR(Auto),
    MAKE_PAIR(Scroll),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Visibility, Value)
  lua.DEFINE_ENUM(Visibility, {
    MAKE_PAIR(Visible),
    MAKE_PAIR(Hidden),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FontStyle, Value)
  lua.DEFINE_ENUM(FontStyle, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(Italic),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FontWeight, Value)
  lua.DEFINE_ENUM(FontWeight, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(Normal),
    MAKE_PAIR(Bold),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextAlign, Value)
  lua.DEFINE_ENUM(TextAlign, {
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
    MAKE_PAIR(Center),
    MAKE_PAIR(Justify),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextDecoration, Value)
  lua.DEFINE_ENUM(TextDecoration, {
    MAKE_PAIR(None),
    MAKE_PAIR(Underline),
    MAKE_PAIR(Overline),
    MAKE_PAIR(LineThrough),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextTransform, Value)
  lua.DEFINE_ENUM(TextTransform, {
    MAKE_PAIR(None),
    MAKE_PAIR(Capitalize),
    MAKE_PAIR(Uppercase),
    MAKE_PAIR(Lowercase),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(WhiteSpace, Value)
  lua.DEFINE_ENUM(WhiteSpace, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(Pre),
    MAKE_PAIR(Nowrap),
    MAKE_PAIR(Prewrap),
    MAKE_PAIR(Preline),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(WordBreak, Value)
  lua.DEFINE_ENUM(WordBreak, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(BreakAll),
    MAKE_PAIR(BreakWord),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Drag, Value)
  lua.DEFINE_ENUM(Drag, {
    MAKE_PAIR(None),
    MAKE_PAIR(Drag),
    MAKE_PAIR(DragDrop),
    MAKE_PAIR(Block),
    MAKE_PAIR(Clone),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TabIndex, Value)
  lua.DEFINE_ENUM(TabIndex, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Focus, Value)
  lua.DEFINE_ENUM(Focus, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OverscrollBehavior, Value)
  lua.DEFINE_ENUM(OverscrollBehavior, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(Contain),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(PointerEvents, Value)
  lua.DEFINE_ENUM(PointerEvents, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OriginX, Value)
  lua.DEFINE_ENUM(OriginX, {
    MAKE_PAIR(Left),
    MAKE_PAIR(Center),
    MAKE_PAIR(Right),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OriginY, Value)
  lua.DEFINE_ENUM(OriginY, {
    MAKE_PAIR(Top),
    MAKE_PAIR(Center),
    MAKE_PAIR(Bottom),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignContent, Value)
  lua.DEFINE_ENUM(AlignContent, {
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(SpaceBetween),
    MAKE_PAIR(SpaceAround),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignItems, Value)
  lua.DEFINE_ENUM(AlignItems, {
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(Baseline),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignSelf, Value)
  lua.DEFINE_ENUM(AlignSelf, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(Baseline),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FlexDirection, Value)
  lua.DEFINE_ENUM(FlexDirection, {
    MAKE_PAIR(Row),
    MAKE_PAIR(RowReverse),
    MAKE_PAIR(Column),
    MAKE_PAIR(ColumnReverse),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FlexWrap, Value)
  lua.DEFINE_ENUM(FlexWrap, {
    MAKE_PAIR(Nowrap),
    MAKE_PAIR(Wrap),
    MAKE_PAIR(WrapReverse),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(JustifyContent, Value)
  lua.DEFINE_ENUM(JustifyContent, {
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(SpaceBetween),
    MAKE_PAIR(SpaceAround),
  });
#undef MAKE_PAIR
  // clang-format on
}

void registerBox(sol::table &lua) {
  // clang-format off
#define MAKE_PAIR(Value) _MAKE_PAIR(BoxArea, Value)
  lua.DEFINE_ENUM(BoxArea, {
    MAKE_PAIR(Margin),
    MAKE_PAIR(Border),
    MAKE_PAIR(Padding),
    MAKE_PAIR(Content),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(BoxEdge, Value)
  lua.DEFINE_ENUM(BoxEdge, {
    MAKE_PAIR(Top),
    MAKE_PAIR(Right),
    MAKE_PAIR(Bottom),
    MAKE_PAIR(Left),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(BoxDirection, Value)
  lua.DEFINE_ENUM(BoxDirection, {
    MAKE_PAIR(Vertical),
    MAKE_PAIR(Horizontal),
  });
#undef MAKE_PAIR

  lua.DEFINE_USERTYPE(Box,
    sol::call_constructor,
    sol::factories(
      []{ return Box{}; },
      [](const glm::vec2 content){ return Box{to_Rml(content)}; }
    ),
    
    sol::meta_function::equal_to, &Box::operator==,

    "getPosition", sol::overload(
      [](const Box &self) { return to_glm(self.GetPosition()); },
      [](const Box &self, BoxArea area) {
        return to_glm(self.GetPosition(area));
      }
    ),
    "getSize", [](const Box &self, BoxArea area) {
      return to_glm(self.GetSize(area));
    },
    "setContent", [](Box &self, const glm::vec2 content) {
      self.SetContent(to_Rml(content));
    },
    "setEdge", &Box::SetEdge,
    "getEdge", &Box::GetEdge,
    "getCumulativeEdge", &Box::GetCumulativeEdge,
    "getSizeAcross", sol::overload(
      [](const Box &self, BoxDirection direction, BoxArea area) {
        return self.GetSizeAcross(direction, area);
      },
      &Box::GetSizeAcross
    )
  );
  // clang-format on
}
void registerStyleSheet(sol::table &lua) {
  lua.DEFINE_USERTYPE(StyleSheet, sol::no_constructor);
}
void registerStyleSheetSpecification(sol::table &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(StyleSheetSpecification,
    sol::no_constructor,

    "parsePropertyDeclaration", StyleSheetSpecification::ParsePropertyDeclaration,
  
    "getPropertyId", StyleSheetSpecification::GetPropertyId,
    "getShorthandId", StyleSheetSpecification::GetShorthandId,
    "getPropertyName", StyleSheetSpecification::GetPropertyName,
    "getShorthandName", StyleSheetSpecification::GetShorthandName
  );
  // clang-format on
}

} // namespace

void registerStyle(sol::table &lua) {
  auto m = lua["Style"].get_or_create<sol::table>();
  registerStyleTypes(m);

  registerBox(lua);
  registerStyleSheet(lua);
  registerStyleSheetSpecification(lua);
}
