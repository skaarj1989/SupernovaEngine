add_library(Camera
  "include/PerspectiveCamera.hpp"
  "src/PerspectiveCamera.cpp"
  "include/ClippingPlanes.hpp"
  "src/ClippingPlanes.cpp"
)
target_include_directories(Camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Camera PUBLIC Common Transform)
set_target_properties(Camera PROPERTIES FOLDER "Framework")
set_warning_level(Camera)
enable_profiler(Camera PRIVATE)
