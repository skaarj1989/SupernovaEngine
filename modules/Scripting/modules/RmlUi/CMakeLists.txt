find_package(EnTT REQUIRED)
find_package(RmlUi REQUIRED)

add_library(LuaRmlUi
  "include/LuaRmlUi.hpp"
  "src/LuaRmlUi.cpp"

  "src/Rml2glm.hpp"
  "src/glm2Rml.hpp"

  "src/Context.hpp"
  "src/Context.cpp"
  "src/Debugger.hpp"
  "src/Debugger.cpp"
  "src/IDs.hpp"
  "src/IDs.cpp"
  "src/Event.hpp"
  "src/Event.cpp"
  "src/EventListener.hpp"
  "src/DataModel.hpp"
  "src/DataModel.cpp"
  "src/TransformPrimitives.hpp"
  "src/TransformPrimitives.cpp"
  "src/Document.hpp"
  "src/Document.cpp"
  "src/Element.hpp"
  "src/Element.cpp"
  "src/Style.hpp"
  "src/Style.cpp"
  "src/Property.hpp"
  "src/Property.cpp"
  "src/Tween.hpp"
  "src/Tween.cpp"
)
target_include_directories(LuaRmlUi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaRmlUi
  PRIVATE RmlUiPlatformInterface HelperMacros
  PUBLIC EnTT::EnTT ScriptTypeInfo Math RmlCore
)
set_target_properties(LuaRmlUi PROPERTIES FOLDER "Framework/ScriptingModules")
set_warning_level(LuaRmlUi)

if(MSVC)
  target_compile_options(LuaRmlUi PRIVATE /bigobj)
endif()

add_binding("LuaRmlUi.hpp" "registerRmlUi")
