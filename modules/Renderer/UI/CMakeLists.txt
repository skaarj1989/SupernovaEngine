add_resources(
  TARGET UIShaders
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  OUT_DIR shaders
)

add_subdirectory(ImGuiRenderer)
add_subdirectory(NuklearRenderer)
add_subdirectory(RmlUiRenderer)
