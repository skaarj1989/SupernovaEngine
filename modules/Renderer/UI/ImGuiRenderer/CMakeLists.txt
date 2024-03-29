add_library(ImGuiRenderer
  "include/ImGuiRenderer.hpp"
  "src/ImGuiRenderer.cpp"
  "include/ImGuiRenderInterface.hpp"
  "src/ImGuiRenderInterface.cpp"
)
target_include_directories(ImGuiRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ImGuiRenderer
  PRIVATE ShaderCodeBuilder
  PUBLIC VulkanRHI imgui::imgui
)
set_target_properties(ImGuiRenderer PROPERTIES FOLDER "Framework/Renderer/")
set_warning_level(ImGuiRenderer)

add_dependencies(ImGuiRenderer Copy-UIShaders)
