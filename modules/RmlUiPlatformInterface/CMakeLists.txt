find_package(RmlUi CONFIG REQUIRED)

add_library(RmlUiPlatformInterface
  "include/RmlUiPlatformInterface.hpp"
  "src/RmlUiPlatformInterface.cpp"
)
target_include_directories(RmlUiPlatformInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RmlUiPlatformInterface
  PRIVATE FileSystem
  PUBLIC RmlCore RmlDebugger
)
set_target_properties(RmlUiPlatformInterface PROPERTIES FOLDER "Framework/GUI")
