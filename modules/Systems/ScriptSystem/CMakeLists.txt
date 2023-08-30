find_package(spdlog REQUIRED)

add_library(ScriptSystem "include/ScriptSystem.hpp" "src/ScriptSystem.cpp")
target_include_directories(ScriptSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ScriptSystem
  PRIVATE spdlog::spdlog
  PUBLIC SystemCommon ScriptComponent
)
set_target_properties(ScriptSystem PROPERTIES FOLDER "Framework/Systems")
