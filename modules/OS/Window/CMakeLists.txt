find_package(EnTT REQUIRED)

add_library(Window
  "include/os/Window.hpp"
  "src/WindowBuilder.cpp"
  "src/Window.cpp"
  "include/os/WindowEvents.hpp"
  "include/os/Monitor.hpp"
)
target_include_directories(Window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Window PUBLIC EnTT::EnTT Platform InputEvents)
set_target_properties(Window PROPERTIES FOLDER "Framework/OS")
set_warning_level(Window)

if(WIN32)
  target_sources(Window PRIVATE "src/Win32Window.cpp" "src/Win32Monitor.cpp")
elseif(LINUX)
  target_sources(Window PRIVATE "src/XWindow.cpp" "src/XMonitor.cpp")
endif()

enable_profiler(Window PRIVATE)
