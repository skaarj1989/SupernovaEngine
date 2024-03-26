function(DEFINE_SCRIPT_MODULE)
  cmake_parse_arguments(PARSE_ARGV 0 ARGS "" "TARGET" "SOURCES;LIBRARIES")

  if(TARGET ${ARGS_TARGET})
    set(TARGET_NAME Lua${ARGS_TARGET})
    add_library(${TARGET_NAME} ${ARGS_SOURCES})
    target_include_directories(${TARGET_NAME}
      PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(${TARGET_NAME}
      PRIVATE HelperMacros
      PUBLIC ${ARGS_TARGET} ${ARGS_LIBRARIES} MetaHelper sol2::sol2
    )
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Framework/ScriptingModules")
  endif()
endfunction()

function(ADD_BINDING HEADER_FILE REGISTER_FUNCTION)
  set_property(GLOBAL APPEND PROPERTY LUA_MODULE_HEADER_FILES ${HEADER_FILE})
  set_property(GLOBAL APPEND PROPERTY LUA_MODULE_REGISTRATION_FUNCTIONS ${REGISTER_FUNCTION})
endfunction()

function(CREATE_LUA_BINDINGS_HEADER_FILE)
  get_property(HEADER_FILE_LIST GLOBAL PROPERTY LUA_MODULE_HEADER_FILES)
  get_property(FUNCTION_LIST GLOBAL PROPERTY LUA_MODULE_REGISTRATION_FUNCTIONS)

  set(SOURCE_CODE "#pragma once\n\n// Auto-generated file. Do not edit.\n\n")

  foreach(HEADER_FILE IN LISTS HEADER_FILE_LIST)
    string(APPEND SOURCE_CODE "#if __has_include(\"${HEADER_FILE}\")\n# include \"${HEADER_FILE}\"\n#endif\n")
  endforeach()

  string(APPEND SOURCE_CODE "\ninline void registerModules(sol::state &lua) {\n")

  foreach(HEADER_FILE REGISTER_FUNCTION IN ZIP_LISTS HEADER_FILE_LIST FUNCTION_LIST)
    string(APPEND SOURCE_CODE "#if __has_include(\"${HEADER_FILE}\")\n  ${REGISTER_FUNCTION}(lua)\;\n#endif\n")
  endforeach()

  string(APPEND SOURCE_CODE "}\n")

  file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../include/LuaModules.hpp ${SOURCE_CODE})
endfunction()
