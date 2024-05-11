find_package(glm REQUIRED)

add_library(CameraController
  "include/CameraController.hpp"
  "src/CameraController.cpp"
)
target_include_directories(CameraController
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(CameraController
  PUBLIC glm::glm
  PRIVATE Transform Camera imgui::imgui
)
set_target_properties(CameraController PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(CameraController)
