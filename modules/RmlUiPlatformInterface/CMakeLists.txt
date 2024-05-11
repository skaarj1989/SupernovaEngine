FetchContent_Declare(RmlUi
  GIT_REPOSITORY https://github.com/mikke89/RmlUi
  GIT_TAG master
  OVERRIDE_FIND_PACKAGE
)

# RmlUI uses older robin_hood (3.9.0)
# The new version (3.11.5) from vcpkg, causes segfault in: nodesDoNotDeallocate
set(NO_THIRDPARTY_CONTAINERS ON CACHE BOOL "")
set(BUILD_SAMPLES OFF CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")
FetchContent_MakeAvailable(RmlUi)

set_target_properties(rmlui_core rmlui_debugger PROPERTIES FOLDER "Third-party/RmlUi")

add_library(RmlUiPlatformInterface
  "include/RmlUiPlatformInterface.hpp"
  "src/RmlUiPlatformInterface.cpp"
)
target_include_directories(RmlUiPlatformInterface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(RmlUiPlatformInterface
  PRIVATE Common Platform FileSystem
  PUBLIC InputEvents RmlUi::Core RmlUi::Debugger
)
set_target_properties(RmlUiPlatformInterface PROPERTIES FOLDER "Framework/GUI")
set_warning_level(RmlUiPlatformInterface)
