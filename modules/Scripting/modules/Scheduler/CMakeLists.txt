find_package(EnTT REQUIRED)

add_library(LuaScheduler "include/LuaScheduler.hpp" "src/LuaScheduler.cpp")
target_include_directories(LuaScheduler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaScheduler
  PRIVATE HelperMacros
  PUBLIC EnTT::EnTT sol2::sol2
)
set_target_properties(LuaScheduler PROPERTIES FOLDER "Framework/ScriptingModules")

add_binding("LuaScheduler.hpp" "registerScheduler")
