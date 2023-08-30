add_library(DebugDraw "include/DebugDraw.hpp" "src/DebugDraw.cpp")
target_include_directories(DebugDraw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(DebugDraw PUBLIC Math)
set_target_properties(DebugDraw PROPERTIES FOLDER "Framework")
