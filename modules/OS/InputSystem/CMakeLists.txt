add_library(InputSystem
  "include/os/InputSystem.hpp"
  "src/InputSystem.cpp"
  "src/Win32InputSystem.cpp"
)
target_include_directories(InputSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(InputSystem PUBLIC Window)
set_target_properties(InputSystem PROPERTIES FOLDER "Framework/OS")
