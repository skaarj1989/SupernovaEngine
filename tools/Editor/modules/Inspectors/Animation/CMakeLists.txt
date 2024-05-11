add_library(AnimationInspectors
  "include/AnimationInspector.hpp"
  "src/AnimationInspector.cpp"

  "include/SkeletonInspector.hpp"
  "src/SkeletonInspector.cpp"
)
target_include_directories(AnimationInspectors
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(AnimationInspectors PRIVATE Animation imgui::imgui)
set_target_properties(AnimationInspectors PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(AnimationInspectors)
