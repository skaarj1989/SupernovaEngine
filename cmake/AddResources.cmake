# IN_DIR = (input) resources location (absolute path)
# OUT_DIR = (output) subdirectory, relative to build/bin
#
# Example usage:
# add_resources(TARGET Fonts IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fonts OUT_DIR fonts)
# add_dependencies(SomeApp Copy-Fonts)

function(ADD_RESOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 ARGS "" "TARGET;IN_DIR;OUT_DIR" "")

    file(GLOB_RECURSE INPUT_FILES "${ARGS_IN_DIR}/*.*")

    set(OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ARGS_OUT_DIR}")

    foreach(IN_FILE ${INPUT_FILES})
        # Get rid of parent part (from IN_FILE)
        file(RELATIVE_PATH FILENAME "${ARGS_IN_DIR}" "${IN_FILE}")
        set(OUT_FILE "${OUT_DIR}/${FILENAME}")

        add_custom_command(
            OUTPUT ${OUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${IN_FILE} ${OUT_FILE}
            DEPENDS ${IN_FILE}
            COMMENT "Copying: ${IN_FILE}")

        list(APPEND OUT_FILES ${OUT_FILE})
    endforeach()

    set(TARGET_NAME "Copy-${ARGS_TARGET}")
    add_custom_target(${TARGET_NAME} DEPENDS ${OUT_FILES})
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "CustomCommand/CopyResources")
endfunction()
