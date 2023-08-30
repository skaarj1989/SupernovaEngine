find_package(spdlog REQUIRED)

add_library(AnimationSystem "include/AnimationSystem.hpp" "src/AnimationSystem.cpp")
target_include_directories(AnimationSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AnimationSystem
  PRIVATE spdlog::spdlog SkeletonDebugDraw
  PUBLIC SystemCommon Animation WorldRenderer
)
set_target_properties(AnimationSystem PROPERTIES FOLDER "Framework/Systems")
