cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

message(STATUS "TOOLCHAIN: ${CMAKE_TOOLCHAIN_FILE}")

project(SupernovaEngine)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib>") # .lib, .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib>") # .dll, .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>") # .exe

set(Boost_NO_WARN_NEW_VERSIONS 1)

if(${CMAKE_VERSION} VERSION_LESS "3.25.0")
  if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX YES)
  endif()
endif()

if(WIN32)
  message(STATUS "Build for WIN32")
  add_definitions(-DNOMINMAX)
elseif(LINUX)
  message(STATUS "Build for Linux")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")

set(CMAKE_CXX_FLAGS_DISTRIBUTION "/GS- /Gy /O2 /Oi /Ot")
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

function(SET_DEBUGGER_WORKING_DIRECTORY)
  set_target_properties(${ARGV0} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
endfunction()

add_subdirectory(extern)
include(cmake/AddResources.cmake)
include(cmake/GroupProjectFiles.cmake)

add_subdirectory(modules)
add_subdirectory(tools)

add_subdirectory(examples)
