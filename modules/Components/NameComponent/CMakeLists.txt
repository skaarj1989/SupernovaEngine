add_library(NameComponent INTERFACE "NameComponent.hpp")
target_include_directories(NameComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(NameComponent PROPERTIES FOLDER "Framework/Components")
