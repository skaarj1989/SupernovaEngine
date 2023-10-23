add_library(InputSystem
  "include/os/InputSystem.hpp"
  "src/InputSystem.cpp"
)
target_include_directories(InputSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(InputSystem PUBLIC Window)

if(WIN32)
  target_sources(InputSystem PRIVATE "src/Win32InputSystem.cpp")
elseif(LINUX)
  target_sources(InputSystem PRIVATE "src/XInputSystem.cpp")
endif()

set_target_properties(InputSystem PROPERTIES FOLDER "Framework/OS")
