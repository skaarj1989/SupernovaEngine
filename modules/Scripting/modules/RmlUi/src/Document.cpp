#include "Document.hpp"
#include "RmlUi/Core/Context.h"
#include "RmlUi/Core/StyleSheet.h"
#include "RmlUi/Core/ElementDocument.h"

#include "Sol2HelperMacros.hpp"

using namespace Rml;

void registerDocument(sol::table &lua) {
  // clang-format off
#define MAKE_PAIR(Value) _MAKE_PAIR(ModalFlag, Value)
  DEFINE_ENUM(ModalFlag, {
    MAKE_PAIR(None),
    MAKE_PAIR(Modal),
    MAKE_PAIR(Keep),
  });
#undef MAKE_PAIR
#define MAKE_PAIR(Value) _MAKE_PAIR(FocusFlag, Value)
  DEFINE_ENUM(FocusFlag, {
    MAKE_PAIR(None),
    MAKE_PAIR(Document),
    MAKE_PAIR(Keep),
    MAKE_PAIR(Auto),
  });
#undef MAKE_PAIR

  DEFINE_USERTYPE(ElementDocument,
    sol::no_constructor,
    sol::base_classes, sol::bases<Element>(),

    "getContext", &ElementDocument::GetContext,

    "setTitle", &ElementDocument::SetTitle,
    "getTitle", &ElementDocument::GetTitle,

    "getSourceURL", &ElementDocument::GetSourceURL,
    
    "getStyleSheet", &ElementDocument::GetStyleSheet,
    "reloadStyleSheet", &ElementDocument::ReloadStyleSheet,

    "pullToFront", &ElementDocument::PullToFront,
    "pushToBack", &ElementDocument::PushToBack,

    "show", sol::overload(
      [](ElementDocument &self) { self.Show(); },
      [](ElementDocument &self, ModalFlag modalFlag) {
        self.Show(modalFlag);
      },
      &ElementDocument::Show
    ),
    "hide", &ElementDocument::Hide,
    "close", &ElementDocument::Close,

    "createElement", &ElementDocument::CreateElement,
    "createTextNode", &ElementDocument::CreateTextNode,

    "isModal", &ElementDocument::IsModal,

    "updateDocument", &ElementDocument::UpdateDocument,

    sol::meta_function::to_string, [](const ElementDocument &self) {
      return std::format("Document({})", self.GetTitle());
    }
  );
  // clang-format on
}
