add_library(SkeletonDebugDraw "include/SkeletonDebugDraw.hpp" "src/SkeletonDebugDraw.cpp")
target_include_directories(SkeletonDebugDraw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SkeletonDebugDraw PUBLIC Animation DebugDraw)
set_target_properties(SkeletonDebugDraw PROPERTIES FOLDER "Framework")
