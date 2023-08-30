add_library(LuaEntity "include/LuaEntity.hpp" "src/LuaEntity.cpp")
target_include_directories(LuaEntity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaEntity
  PRIVATE HelperMacros
  PUBLIC EnTT::EnTT MetaHelper MetaComponent ScriptTypeInfo HierarchySystem
)
set_target_properties(LuaEntity PROPERTIES FOLDER "Framework/ScriptingModules")
