add_library(NuklearRenderer "include/NuklearRenderer.hpp" "src/NuklearRenderer.cpp")
target_compile_definitions(NuklearRenderer
  PRIVATE NK_INCLUDE_DEFAULT_ALLOCATOR NK_INCLUDE_VERTEX_BUFFER_OUTPUT)
target_include_directories(NuklearRenderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NuklearRenderer
  PRIVATE ShaderCodeBuilder
  PUBLIC VulkanRHI Nuklear
)
set_target_properties(NuklearRenderer PROPERTIES FOLDER "Framework/Renderer/")
set_warning_level(NuklearRenderer)

add_dependencies(NuklearRenderer Copy-UIShaders)
