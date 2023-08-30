add_library(ImGuiPlatformInterface
  "include/ImGuiPlatformInterface.hpp"
  "src/ImGuiPlatformInterface.cpp"
)
target_include_directories(ImGuiPlatformInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ImGuiPlatformInterface
  PRIVATE imgui::imgui
  PUBLIC Window
)
set_target_properties(ImGuiPlatformInterface PROPERTIES FOLDER "Framework/GUI")
