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
  DEFINE_ENUM(Display, {
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
  DEFINE_ENUM(Position, {
    MAKE_PAIR(Static),
    MAKE_PAIR(Relative),
    MAKE_PAIR(Absolute),
    MAKE_PAIR(Fixed),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Float, Value)
  DEFINE_ENUM(Float, {
    MAKE_PAIR(None),
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Clear, Value)
  DEFINE_ENUM(Clear, {
    MAKE_PAIR(None),
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
    MAKE_PAIR(Both),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(BoxSizing, Value)
  DEFINE_ENUM(BoxSizing, {
    MAKE_PAIR(ContentBox),
    MAKE_PAIR(BorderBox),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Overflow, Value)
  DEFINE_ENUM(Overflow, {
    MAKE_PAIR(Visible),
    MAKE_PAIR(Hidden),
    MAKE_PAIR(Auto),
    MAKE_PAIR(Scroll),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Visibility, Value)
  DEFINE_ENUM(Visibility, {
    MAKE_PAIR(Visible),
    MAKE_PAIR(Hidden),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FontStyle, Value)
  DEFINE_ENUM(FontStyle, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(Italic),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FontWeight, Value)
  DEFINE_ENUM(FontWeight, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(Normal),
    MAKE_PAIR(Bold),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextAlign, Value)
  DEFINE_ENUM(TextAlign, {
    MAKE_PAIR(Left),
    MAKE_PAIR(Right),
    MAKE_PAIR(Center),
    MAKE_PAIR(Justify),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextDecoration, Value)
  DEFINE_ENUM(TextDecoration, {
    MAKE_PAIR(None),
    MAKE_PAIR(Underline),
    MAKE_PAIR(Overline),
    MAKE_PAIR(LineThrough),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TextTransform, Value)
  DEFINE_ENUM(TextTransform, {
    MAKE_PAIR(None),
    MAKE_PAIR(Capitalize),
    MAKE_PAIR(Uppercase),
    MAKE_PAIR(Lowercase),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(WhiteSpace, Value)
  DEFINE_ENUM(WhiteSpace, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(Pre),
    MAKE_PAIR(Nowrap),
    MAKE_PAIR(Prewrap),
    MAKE_PAIR(Preline),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(WordBreak, Value)
  DEFINE_ENUM(WordBreak, {
    MAKE_PAIR(Normal),
    MAKE_PAIR(BreakAll),
    MAKE_PAIR(BreakWord),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Drag, Value)
  DEFINE_ENUM(Drag, {
    MAKE_PAIR(None),
    MAKE_PAIR(Drag),
    MAKE_PAIR(DragDrop),
    MAKE_PAIR(Block),
    MAKE_PAIR(Clone),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(TabIndex, Value)
  DEFINE_ENUM(TabIndex, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(Focus, Value)
  DEFINE_ENUM(Focus, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OverscrollBehavior, Value)
  DEFINE_ENUM(OverscrollBehavior, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(Contain),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(PointerEvents, Value)
  DEFINE_ENUM(PointerEvents, {
    MAKE_PAIR(None),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OriginX, Value)
  DEFINE_ENUM(OriginX, {
    MAKE_PAIR(Left),
    MAKE_PAIR(Center),
    MAKE_PAIR(Right),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(OriginY, Value)
  DEFINE_ENUM(OriginY, {
    MAKE_PAIR(Top),
    MAKE_PAIR(Center),
    MAKE_PAIR(Bottom),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignContent, Value)
  DEFINE_ENUM(AlignContent, {
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(SpaceBetween),
    MAKE_PAIR(SpaceAround),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignItems, Value)
  DEFINE_ENUM(AlignItems, {
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(Baseline),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(AlignSelf, Value)
  DEFINE_ENUM(AlignSelf, {
    MAKE_PAIR(Auto),
    MAKE_PAIR(FlexStart),
    MAKE_PAIR(FlexEnd),
    MAKE_PAIR(Center),
    MAKE_PAIR(Baseline),
    MAKE_PAIR(Stretch),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FlexDirection, Value)
  DEFINE_ENUM(FlexDirection, {
    MAKE_PAIR(Row),
    MAKE_PAIR(RowReverse),
    MAKE_PAIR(Column),
    MAKE_PAIR(ColumnReverse),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(FlexWrap, Value)
  DEFINE_ENUM(FlexWrap, {
    MAKE_PAIR(Nowrap),
    MAKE_PAIR(Wrap),
    MAKE_PAIR(WrapReverse),
  });
#undef MAKE_PAIR

#define MAKE_PAIR(Value) _MAKE_PAIR(JustifyContent, Value)
  DEFINE_ENUM(JustifyContent, {
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
  using BoxArea = Box::Area;
#define MAKE_PAIR(Value) _MAKE_PAIR(BoxArea, Value)
  DEFINE_ENUM(BoxArea, {
    MAKE_PAIR(MARGIN),
    MAKE_PAIR(BORDER),
    MAKE_PAIR(PADDING),
    MAKE_PAIR(CONTENT),
    MAKE_PAIR(NUM_AREAS),
  });
#undef MAKE_PAIR

  using BoxEdge = Box::Edge;
#define MAKE_PAIR(Value) _MAKE_PAIR(BoxEdge, Value)
  DEFINE_ENUM(BoxEdge, {
    MAKE_PAIR(TOP),
    MAKE_PAIR(RIGHT),
    MAKE_PAIR(BOTTOM),
    MAKE_PAIR(LEFT),
    MAKE_PAIR(NUM_EDGES),
  });
#undef MAKE_PAIR

  using BoxDirection = Box::Direction;
#define MAKE_PAIR(Value) _MAKE_PAIR(BoxDirection, Value)
  DEFINE_ENUM(BoxDirection, {
    MAKE_PAIR(VERTICAL),
    MAKE_PAIR(HORIZONTAL),
  });
#undef MAKE_PAIR

  DEFINE_USERTYPE(Box,
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
  DEFINE_USERTYPE(StyleSheet, sol::no_constructor);
}
void registerStyleSheetSpecification(sol::table &lua) {
  // clang-format off
  DEFINE_USERTYPE(StyleSheetSpecification,
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
