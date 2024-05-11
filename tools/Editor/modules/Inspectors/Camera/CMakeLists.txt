add_library(CameraInspector
  "include/CameraInspector.hpp"
  "src/CameraInspector.cpp"
)
target_include_directories(CameraInspector
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(CameraInspector
  PRIVATE Camera imgui::imgui TransformInspector
)
set_target_properties(CameraInspector PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(CameraInspector)
