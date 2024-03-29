find_package(RmlUi REQUIRED)

add_library(RmlUiRenderer
  "include/RmlUiRenderer.hpp"
  "src/RmlUiRenderer.cpp"
  "include/RmlUiRenderInterface.hpp"
  "src/RmlUiRenderInterface.cpp"
)
target_include_directories(RmlUiRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RmlUiRenderer
  PRIVATE ShaderCodeBuilder FileSystem STBImageLoader
  PUBLIC RmlCore VulkanRHI
)
set_target_properties(RmlUiRenderer PROPERTIES FOLDER "Framework/Renderer/")
set_warning_level(RmlUiRenderer)

add_resources(
  TARGET RmlUIShaders
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  OUT_DIR shaders
)
add_dependencies(RmlUiRenderer Copy-RmlUIShaders)
