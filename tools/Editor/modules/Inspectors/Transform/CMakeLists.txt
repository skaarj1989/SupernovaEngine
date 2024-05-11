find_package(glm REQUIRED)

add_library(TransformInspector
  "include/TransformInspector.hpp"
  "src/TransformInspector.cpp"
)
target_include_directories(TransformInspector
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(TransformInspector PUBLIC glm::glm PRIVATE Transform ImGuiEx)
set_target_properties(TransformInspector PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(TransformInspector)
