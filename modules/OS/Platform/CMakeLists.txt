find_package(glm REQUIRED)

add_library(Platform "include/os/Platform.hpp")
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Platform PUBLIC glm::glm)

if(WIN32)
  target_sources(Platform PRIVATE "src/Win32Platform.cpp")
elseif(LINUX)
  target_sources(Platform PRIVATE "src/XPlatform.cpp" "include/os/X11.hpp" "src/X11.cpp")

  find_package(X11 REQUIRED)
  target_link_libraries(Platform PRIVATE
    X11::xcb
    X11::xcb_util
    X11::xcb_icccm
    X11::xcb_ewmh
    X11::xcb_xfixes
    X11::xcb_keysyms
    X11::xcb_randr
    X11::X11_xcb
  )
endif()

set_target_properties(Platform PROPERTIES FOLDER "Framework/OS")
set_warning_level(Platform)
