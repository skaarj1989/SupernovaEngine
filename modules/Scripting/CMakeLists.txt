find_package(Lua CONFIG REQUIRED)
find_package(spdlog REQUIRED)

FetchContent_Declare(sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG develop
)
FetchContent_MakeAvailable(sol2)
target_compile_definitions(sol2 INTERFACE SOL_ALL_SAFETIES_ON=1)
target_include_directories(sol2 INTERFACE ${LUA_INCLUDE_DIR})
target_link_libraries(sol2 INTERFACE ${LUA_LIBRARIES})

add_library(Scripting
  "include/ScriptResource.hpp"
  "include/ScriptResourceHandle.hpp"

  "include/ScriptLoader.hpp"
  "src/ScriptLoader.cpp"
  "include/ScriptManager.hpp"
  "src/ScriptManager.cpp"

  "include/ScriptNode.hpp"
  "src/ScriptNode.cpp"

  "include/ScriptContext.hpp"
  "src/ScriptContext.cpp"
)
target_include_directories(Scripting PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Scripting
  PRIVATE spdlog::spdlog
  PUBLIC Resource InputEvents sol2::sol2
)
set_target_properties(Scripting PROPERTIES FOLDER "Framework")
set_warning_level(Scripting)

add_resources(
  TARGET ScriptLibrary
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lua
  OUT_DIR lua
)
add_dependencies(Scripting Copy-ScriptLibrary)

include(cmake/DefineScriptModule.cmake)
