find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(robin_hood CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(renderdoc)

include(FetchContent)
FetchContent_Declare(Tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy
  GIT_TAG v0.10
)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Tracy)

set_target_properties(TracyClient PROPERTIES FOLDER "Third-party/Profiler")

function(ENABLE_PROFILER TARGET)
  get_target_property(TRACY_INCLUDES Tracy::TracyClient INCLUDE_DIRECTORIES)
  target_include_directories(${TARGET} PUBLIC ${TRACY_INCLUDES})
  target_link_libraries(${TARGET}
    PUBLIC $<$<CONFIG:Release>:Tracy::TracyClient>
  )
endfunction()

add_subdirectory(imgui)
add_subdirectory(Nuklear)

set_target_properties(imgui Nuklear PROPERTIES FOLDER "Third-party/GUI")
