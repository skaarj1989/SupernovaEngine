find_package(spdlog REQUIRED)

FetchContent_Declare(ozz-animation
  GIT_REPOSITORY https://github.com/guillaumeblanc/ozz-animation.git
  GIT_TAG master
)

set(ozz_build_tools OFF CACHE BOOL "")
set(ozz_build_samples OFF CACHE BOOL "")
set(ozz_build_howtos OFF CACHE BOOL "")
set(ozz_build_tests OFF CACHE BOOL "")

FetchContent_MakeAvailable(ozz-animation)

set_target_properties(
  BUILD_FUSE_ozz_animation
  BUILD_FUSE_ozz_animation_offline
  BUILD_FUSE_ozz_base
  BUILD_FUSE_ozz_geometry
  BUILD_FUSE_ozz_options
  BUILD_FUSE_ALL
  PROPERTIES FOLDER "Third-party/ozz/fuse"
)
set_target_properties(
  ozz_base
  ozz_animation
  ozz_animation_offline
  ozz_geometry
  ozz_options
  PROPERTIES FOLDER "Third-party/ozz"
)
set_target_properties(
  ozz_base
  ozz_animation
  ozz_animation_offline
  ozz_geometry
  ozz_options
  PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

add_library(Animation
  "include/animation/Conversion.hpp"
  "src/Conversion.cpp"

  "src/OzzFile.hpp"
  "src/OzzFile.cpp"
  "src/TryLoad.hpp"

  "include/animation/SkeletonManager.hpp"
  "src/SkeletonManager.cpp"
  "include/animation/SkeletonResource.hpp"
  "src/SkeletonResource.cpp"
  "include/animation/SkeletonResourceHandle.hpp"
  "include/animation/SkeletonLoader.hpp"
  "src/SkeletonLoader.cpp"

  "include/animation/AnimationManager.hpp"
  "src/AnimationManager.cpp"
  "include/animation/AnimationResource.hpp"
  "src/AnimationResource.cpp"
  "include/animation/AnimationResourceHandle.hpp"
  "include/animation/AnimationLoader.hpp"
  "src/AnimationLoader.cpp"

  "include/animation/PlaybackController.hpp"
  "src/PlaybackController.cpp"

  "include/animation/SkeletonComponent.hpp"
  "include/animation/AnimationComponent.hpp"
)

target_include_directories(Animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Animation
  PRIVATE spdlog::spdlog
  PUBLIC Resource ozz_geometry ozz_animation
)
set_target_properties(Animation PROPERTIES FOLDER "Framework")
set_warning_level(Animation)

add_subdirectory(modules)
