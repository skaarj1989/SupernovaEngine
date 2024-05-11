add_library(Services
  "include/Services.hpp"
  "src/Services.cpp"
)
target_include_directories(Services
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(Services
  PRIVATE WorldRenderer Animation JoltPhysics AudioWorld Scripting
)
set_target_properties(Services PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(Services)
