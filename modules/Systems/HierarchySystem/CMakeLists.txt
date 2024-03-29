find_package(spdlog REQUIRED)

add_library(HierarchySystem
  "include/HierarchySystem.hpp"
  "src/HierarchySystem.cpp"
)
target_include_directories(HierarchySystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(HierarchySystem
  PRIVATE spdlog::spdlog Transform
  PUBLIC RelationshipComponents
)
set_target_properties(HierarchySystem PROPERTIES FOLDER "Framework/Systems")
set_warning_level(HierarchySystem)
