find_package(EnTT REQUIRED)

add_library(ScriptEditor
  "include/ScriptEditor.hpp"
  "src/ScriptEditor.cpp"
)
target_include_directories(ScriptEditor
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ScriptEditor PUBLIC EnTT::EnTT ImGuiEx)
set_target_properties(ScriptEditor PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(ScriptEditor)

enable_profiler(ScriptEditor PUBLIC)
