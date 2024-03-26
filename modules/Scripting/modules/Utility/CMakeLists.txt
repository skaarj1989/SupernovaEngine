find_package(EnTT REQUIRED)

add_library(LuaUtility "include/LuaUtility.hpp" "src/LuaUtility.cpp")
target_include_directories(LuaUtility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaUtility
  PRIVATE HelperMacros ScriptTypeInfo
  PUBLIC EnTT::EnTT sol2::sol2
)
set_target_properties(LuaUtility PROPERTIES FOLDER "Framework/ScriptingModules")

add_binding("LuaUtility.hpp" "registerUtility")
