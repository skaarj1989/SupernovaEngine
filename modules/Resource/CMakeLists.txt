find_package(EnTT REQUIRED)

add_library(Resource "include/Resource.hpp" "src/Resource.cpp" "include/LoaderHelper.hpp")
target_include_directories(Resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Resource PUBLIC EnTT::EnTT FileSystem)
set_target_properties(Resource PROPERTIES FOLDER "Framework")
