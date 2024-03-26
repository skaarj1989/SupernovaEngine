find_package(spdlog REQUIRED)

add_library(LuaLogger "include/LuaLogger.hpp" "src/LuaLogger.cpp")
target_include_directories(LuaLogger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(LuaLogger PUBLIC sol2::sol2 spdlog::spdlog)
set_target_properties(LuaLogger PROPERTIES FOLDER "Framework/ScriptingModules")

add_binding("LuaLogger.hpp" "registerLogger")
