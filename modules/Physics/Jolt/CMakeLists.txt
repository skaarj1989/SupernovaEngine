find_package(spdlog REQUIRED)

FetchContent_Declare(Jolt
  GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
  GIT_TAG master
  SOURCE_SUBDIR Build
)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "")
set(TARGET_HELLO_WORLD OFF CACHE BOOL "")
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "")
set(TARGET_SAMPLES OFF CACHE BOOL "")
set(TARGET_VIEWER OFF CACHE BOOL "")
FetchContent_MakeAvailable(Jolt)

target_compile_definitions(Jolt PUBLIC JPH_DEBUG_RENDERER)

if(MSVC)
  target_compile_options(Jolt PRIVATE /WX-)
  set_target_properties(Jolt PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    FOLDER "Third-party/Jolt"
  )
endif()

add_library(JoltPhysics
  "include/physics/JoltPhysics.hpp"
  "src/JoltPhysics.cpp"

  "include/physics/Conversion.hpp"
  "src/Conversion.cpp"

  "include/physics/PhysicsWorld.hpp"
  "src/PhysicsWorld.cpp"
  "include/physics/DebugRenderer.hpp"
  "src/DebugRenderer.cpp"

  "include/physics/ShapeBuilder.hpp"
  "src/ShapeBuilder.cpp"

  "include/physics/ShapeSerialization.hpp"
  "src/ShapeSerialization.cpp"

  "include/physics/Collider.hpp"
  "include/physics/ColliderResource.hpp"
  "src/ColliderResource.cpp"
  "include/physics/ColliderResourceHandle.hpp"
  "include/physics/ColliderLoader.hpp"
  "src/ColliderLoader.cpp"
  "include/physics/ColliderManager.hpp"
  "src/ColliderManager.cpp"

  "include/physics/CollisionLayer.hpp"
  "include/physics/Events.hpp"

  "include/physics/RigidBody.hpp"
  "src/RigidBody.cpp"

  "include/physics/Character.hpp"
  "src/Character.cpp"
  "include/physics/CharacterVirtual.hpp"
  "src/CharacterVirtual.cpp"
)

target_include_directories(JoltPhysics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(JoltPhysics
  PRIVATE spdlog::spdlog
  PUBLIC Common Math Resource Transform DebugDraw Jolt
)
set_target_properties(JoltPhysics PROPERTIES FOLDER "Framework")

enable_profiler(JoltPhysics PRIVATE)
