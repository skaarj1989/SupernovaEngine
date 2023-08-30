add_library(ScriptComponent INTERFACE "ScriptComponent.hpp")
target_include_directories(ScriptComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ScriptComponent INTERFACE Scripting)
set_target_properties(ScriptComponent PROPERTIES FOLDER "Framework/Components")
