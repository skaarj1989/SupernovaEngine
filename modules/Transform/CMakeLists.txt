add_library(Transform "include/Transform.hpp" "src/Transform.cpp")
target_include_directories(Transform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Transform PUBLIC Math)
set_target_properties(Transform PROPERTIES FOLDER "Framework")
