function(GROUP_MSVC_PROJECT)
  cmake_parse_arguments(PARSE_ARGV 0 ARGS "" "TARGET" "")

  get_target_property(TARGET_SOURCES ${ARGS_TARGET} SOURCES)

  # https://stackoverflow.com/a/33813154/4265215
  foreach(IN_FILE IN ITEMS ${TARGET_SOURCES})
    get_filename_component(FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${IN_FILE}" PATH)
    file(RELATIVE_PATH FILE_RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${FILE_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "${FILE_RELATIVE_PATH}")
    source_group("${GROUP_PATH}" FILES "${IN_FILE}")
  endforeach()
endfunction()
