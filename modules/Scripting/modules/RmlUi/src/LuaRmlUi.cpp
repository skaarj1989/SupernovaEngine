#include "LuaRmlUi.hpp"
#include "sol/state.hpp"
#include "RmlUi/Core/Core.h"
#include "RmlUi/Core/Context.h"

#include "RmlUiPlatformInterface.hpp"

#include "Context.hpp"
#include "Debugger.hpp"
#include "IDs.hpp"
#include "Event.hpp"
#include "DataModel.hpp"
#include "TransformPrimitives.hpp"
#include "Document.hpp"
#include "Element.hpp"
#include "Style.hpp"
#include "Property.hpp"
#include "Tween.hpp"

#include "glm/ext/vector_int2.hpp"
#include "glm/ext/vector_float2.hpp"

namespace {

using namespace Rml;

void registerFreeFunctions(sol::table &lua) {
  lua.set_function("loadFontFace",
                   [](const String &path) { return LoadFontFace(path); });

  lua.set_function("getContext",
                   sol::resolve<Context *(const String &)>(GetContext));
  lua.set_function("getNumContexts", GetNumContexts);

  lua.set_function("processEvent", processEvent);
  lua.set_function("scaleMousePosition",
                   [](const glm::ivec2 mousePos,
                      const glm::ivec2 targetTextureExtent,
                      const glm::ivec2 uiDimensions) {
                     const auto ratio =
                       glm::vec2{uiDimensions} / glm::vec2{targetTextureExtent};
                     auto scaledMousePos = glm::vec2{mousePos} * ratio;
                     return glm::ivec2{scaledMousePos};
                   });
}

} // namespace

void registerRmlUi(sol::state &lua) {
  auto m = lua["ui"].get_or_create<sol::table>();

  registerContext(m);
  registerDebugger(m);
  registerIDs(m);
  registerEvent(m);
  registerDataModel(m);
  registerTransform(m);
  registerDocument(m);
  registerElement(m);
  registerStyle(m);
  registerProperty(m);
  registerTween(m);

  registerFreeFunctions(m);
}
