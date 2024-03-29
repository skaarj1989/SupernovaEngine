add_executable(RmlUiDemo "main.cpp")
target_link_libraries(RmlUiDemo PUBLIC RmlUiApp)
set_target_properties(RmlUiDemo PROPERTIES FOLDER "Demo")
set_warning_level(RmlUiDemo)

set_debugger_working_directory(RmlUiDemo)

add_resources(
  TARGET RmlUiDemoAssets
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets
  OUT_DIR demo-assets
)
add_dependencies(RmlUiDemo Copy-RmlUiDemoAssets)
