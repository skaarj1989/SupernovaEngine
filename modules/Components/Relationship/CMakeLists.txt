find_package(EnTT REQUIRED)

add_library(RelationshipComponents INTERFACE "ParentComponent.hpp" "ChildrenComponent.hpp")
target_include_directories(RelationshipComponents INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RelationshipComponents INTERFACE EnTT::EnTT)
set_target_properties(RelationshipComponents PROPERTIES FOLDER "Framework/Components")
