find_package(EnTT REQUIRED)

add_library(MetaHelper INTERFACE "include/MetaHelper.hpp")
target_include_directories(MetaHelper INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(MetaHelper INTERFACE EnTT::EnTT)
set_target_properties(MetaHelper PROPERTIES FOLDER "Framework")
