find_package(spdlog REQUIRED)

add_library(RenderSystem "include/RenderSystem.hpp" "src/RenderSystem.cpp")
target_include_directories(RenderSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RenderSystem
  PRIVATE spdlog::spdlog
  PUBLIC SystemCommon CameraComponent
)
set_target_properties(RenderSystem PROPERTIES FOLDER "Framework/Systems")
set_warning_level(RenderSystem)
