add_library(ResourceInspector
  "include/ResourceInspector.hpp"
  "src/ResourceInspector.cpp"
)
target_include_directories(ResourceInspector
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ResourceInspector PRIVATE Resource imgui::imgui)
set_target_properties(ResourceInspector PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(ResourceInspector)
