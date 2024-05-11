find_package(spdlog REQUIRED)

add_library(SceneEditor
  "include/Inspector.hpp"

  "include/SceneEditor.hpp"
  "src/SceneEditor.cpp"

  "src/Inspectors/NameComponent.cpp"
  "src/Inspectors/Transform.cpp"
  "src/Inspectors/Hierarchy.cpp"
  "src/Inspectors/Animation/SkeletonComponent.cpp"
  "src/Inspectors/Animation/AnimationComponent.cpp"
  "src/Inspectors/Animation/PlaybackController.cpp"
  "src/Inspectors/Physics/Character.cpp"
  "src/Inspectors/Physics/Collider.cpp"
  "src/Inspectors/Physics/RigidBody.cpp"
  "src/Inspectors/Renderer/Camera.cpp"
  "src/Inspectors/Renderer/Light.cpp"
  "src/Inspectors/Renderer/Mesh.cpp"
  "src/Inspectors/Audio/ListenerComponentInspector.cpp"
  "src/Inspectors/Audio/AudioComponentInspector.hpp"
  "src/Inspectors/Audio/SoundSourceComponent.cpp"
  "src/Inspectors/Audio/AudioPlayerComponent.cpp"
  "src/Inspectors/Script.cpp"
)
target_include_directories(SceneEditor
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(SceneEditor
  PUBLIC
  MetaComponent
  MetaHelper

  Scene

  RmlUiPlatformInterface

  CameraController
  GizmoController

  PRIVATE
  spdlog::spdlog
  Services
  InputSystem

  ImGuiEx

  TransformInspector
  CameraInspector
  ResourceInspector
  WorldRendererInspectors
  AnimationInspectors
  PhysicsInspectors
)
set_target_properties(SceneEditor PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(SceneEditor)

group_msvc_project(TARGET SceneEditor)
enable_profiler(SceneEditor PUBLIC)
