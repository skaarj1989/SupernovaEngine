add_library(HelperMacros INTERFACE "Sol2HelperMacros.hpp")
target_include_directories(HelperMacros INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(HelperMacros PROPERTIES FOLDER "Framework/ScriptingModules")
