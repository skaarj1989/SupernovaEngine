find_package(EnTT REQUIRED)

add_library(GizmoController
  "include/GizmoController.hpp"
  "src/GizmoController.cpp"
)
target_include_directories(GizmoController
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(GizmoController
  PUBLIC EnTT::EnTT imgui::imgui
  PRIVATE Transform Camera
)
set_target_properties(GizmoController PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(GizmoController)
