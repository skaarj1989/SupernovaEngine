add_library(RmlUiApp "include/RmlUiApp.hpp" "src/RmlUiApp.cpp")
target_include_directories(RmlUiApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RmlUiApp
  PRIVATE FileSystem
  PUBLIC BaseApp RmlUiPlatformInterface RmlUiRenderer
)
set_target_properties(RmlUiApp PROPERTIES FOLDER "Framework/App")
set_warning_level(RmlUiApp)
