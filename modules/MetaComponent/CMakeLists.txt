add_library(MetaComponent INTERFACE "include/MetaComponent.hpp")
target_include_directories(MetaComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(MetaComponent PROPERTIES FOLDER "Framework")
