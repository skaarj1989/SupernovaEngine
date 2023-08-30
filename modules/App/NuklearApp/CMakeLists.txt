add_library(NuklearApp "include/NuklearApp.hpp" "src/NuklearApp.cpp")
target_compile_definitions(NuklearApp
  PRIVATE NK_INCLUDE_DEFAULT_ALLOCATOR NK_INCLUDE_VERTEX_BUFFER_OUTPUT
)
target_include_directories(NuklearApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NuklearApp PUBLIC BaseApp NuklearRenderer)
set_target_properties(NuklearApp PROPERTIES FOLDER "Framework/App")
