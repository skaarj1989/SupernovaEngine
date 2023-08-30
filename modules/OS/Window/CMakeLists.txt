find_package(EnTT REQUIRED)

add_library(Window
  "include/os/Window.hpp"
  "src/WindowBuilder.cpp"
  "src/Window.cpp"
  "src/Win32Window.cpp"
  "include/os/WindowEvents.hpp"
  "include/os/Monitor.hpp"
  "src/Win32Monitor.cpp"
)
target_include_directories(Window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Window PUBLIC EnTT::EnTT InputEvents)
set_target_properties(Window PROPERTIES FOLDER "Framework/OS")
