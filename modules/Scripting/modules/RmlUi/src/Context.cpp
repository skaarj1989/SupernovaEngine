#include "Context.hpp"
#include "RmlUi/Core/Context.h"
#include "RmlUi/Core/ElementDocument.h"
#include "EventListener.hpp"
#include "DataModel.hpp"
#include "glm2Rml.hpp"
#include "Rml2glm.hpp"

#include "Sol2HelperMacros.hpp"
#include <format>

using namespace Rml;

void registerContext(sol::table &lua) {
  // clang-format off
  DEFINE_USERTYPE(Context,
    sol::no_constructor,

    "getName", &Context::GetName,

    "setDimensions", [](Context &self, const glm::ivec2 dimensions) {
      self.SetDimensions(to_Rml(dimensions));
    },
    "getDimensions", [](const Context &self) {
      return to_glm(self.GetDimensions());
    },

    "createDocument", &Context::CreateDocument,
    "loadDocument",
      sol::resolve<ElementDocument *(const String &)>(&Context::LoadDocument),
    "unloadDocument", &Context::UnloadDocument,
    "unloadAllDocuments", &Context::UnloadAllDocuments,

    "activateTheme", &Context::ActivateTheme,
    "isThemeActive", &Context::IsThemeActive,

    "getDocument", sol::overload(
      sol::resolve<ElementDocument *(const String &)>(&Context::GetDocument),
      sol::resolve<ElementDocument *(int32_t)>(&Context::GetDocument)
    ),
    "getNumDocuments", &Context::GetNumDocuments,

    "getHoverElement", &Context::GetHoverElement,
    "getFocusElement", &Context::GetFocusElement,
    "getRootElement", &Context::GetRootElement,

    "getElementAtPoint", [](const Context &self, const glm::vec2 &v) {
      return self.GetElementAtPoint(to_Rml(v));
    },

    "pullDocumentToFront", &Context::PullDocumentToFront,
    "pushDocumentToBack", &Context::PushDocumentToBack,
    "unfocusDocument", &Context::UnfocusDocument,

    "addEventListener", sol::overload(
      [](Context &self, const String &event, sol::table t) {
        return addEventListener(self, event, t);
      },
      [](Context &self, const String &event, sol::table t, bool inCapturePhase) {
        return addEventListener(self, event, t, inCapturePhase);
      }
    ),
    "removeEventListener", sol::overload(
      [](Context &self, const String &event, ScriptEventListener *listener) {
        self.RemoveEventListener(event, listener);
      },
      [](Context &self, const String &event, ScriptEventListener *listener,
         bool inCapturePhase) {
        self.RemoveEventListener(event, listener, inCapturePhase);
      }
    ),
    "isMouseInteracting", &Context::IsMouseInteracting,

    "bindDataModel", [](Context &self, const String &name, sol::table t) {
      return std::make_unique<ScriptDataModel>(self, name, t);
    },

    sol::meta_function::to_string, [](const Context &self) {
      return std::format("Context({})", self.GetName());
    }
  );
  // clang-format on
}
