add_library(LuaDispatcher "include/LuaDispatcher.hpp" "src/LuaDispatcher.cpp")
target_include_directories(LuaDispatcher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaDispatcher
  PRIVATE HelperMacros
  PUBLIC MetaHelper ScriptTypeInfo EnTT::EnTT sol2::sol2
)
set_target_properties(LuaDispatcher PROPERTIES FOLDER "Framework/ScriptingModules")
