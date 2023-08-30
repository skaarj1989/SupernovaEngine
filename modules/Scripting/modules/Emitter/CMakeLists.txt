add_library(LuaEmitter INTERFACE "include/LuaEmitter.hpp")
target_include_directories(LuaEmitter INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaEmitter INTERFACE MetaHelper ScriptTypeInfo)
set_target_properties(LuaEmitter PROPERTIES FOLDER "Framework/ScriptingModules")
