add_library(CameraComponent INTERFACE "CameraComponent.hpp")
target_include_directories(CameraComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CameraComponent INTERFACE WorldRenderer)
set_target_properties(CameraComponent PROPERTIES FOLDER "Framework/Components")
