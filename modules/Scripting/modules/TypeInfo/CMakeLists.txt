add_library(ScriptTypeInfo INTERFACE "include/ScriptTypeInfo.hpp")
target_include_directories(ScriptTypeInfo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ScriptTypeInfo INTERFACE sol2::sol2)
set_target_properties(ScriptTypeInfo PROPERTIES FOLDER "Framework/ScriptingModules")
