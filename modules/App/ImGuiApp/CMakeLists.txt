add_library(ImGuiApp "include/ImGuiApp.hpp" "src/ImGuiApp.cpp")
target_include_directories(ImGuiApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ImGuiApp
  PRIVATE FileSystem ImGuiPlatformInterface
  PUBLIC BaseApp ImGuiRenderer
)
set_target_properties(ImGuiApp PROPERTIES FOLDER "Framework/App")

add_resources(
  TARGET Fonts
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fonts
  OUT_DIR fonts
)
add_dependencies(ImGuiApp Copy-Fonts)
