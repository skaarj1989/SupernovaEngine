add_library(ResourceDockSpace
  "include/ResourceDockSpace.hpp"
  "src/ResourceDockSpace.cpp"

  "src/CacheInspector.hpp"

  "src/MeshCache.hpp"
  "src/MeshCache.cpp"
  "src/MaterialCache.hpp"
  "src/MaterialCache.cpp"
  "src/TextureCache.hpp"
  "src/TextureCache.cpp"

  "src/SkeletonCache.hpp"
  "src/SkeletonCache.cpp"
  "src/AnimationCache.hpp"
  "src/AnimationCache.cpp"

  "src/ColliderCache.hpp"
  "src/ColliderCache.cpp"

  "src/AudioClipCache.hpp"
  "src/AudioClipCache.cpp"

  "src/ScriptCache.hpp"
  "src/ScriptCache.cpp"
)
target_include_directories(ResourceDockSpace
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ResourceDockSpace
  PUBLIC ImGuiEx
  PRIVATE
  Services

  WorldRenderer
  Animation
  JoltPhysics
  AudioWorld
  Scripting

  TransformInspector
  CameraInspector
  ResourceInspector
  WorldRendererInspectors
  AnimationInspectors
  PhysicsInspectors
)
set_target_properties(ResourceDockSpace PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(ResourceDockSpace)

group_msvc_project(TARGET ResourceDockSpace)
