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
