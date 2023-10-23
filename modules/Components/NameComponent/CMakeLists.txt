find_package(EnTT REQUIRED)

add_library(NameComponent INTERFACE "NameComponent.hpp")
target_include_directories(NameComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NameComponent INTERFACE EnTT::EnTT)
set_target_properties(NameComponent PROPERTIES FOLDER "Framework/Components")
