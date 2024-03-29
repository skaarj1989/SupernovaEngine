add_library(Scene
  "include/Scene.hpp"
  "src/Scene.cpp"
  "src/SystemSerializationUtility.hpp"
)
target_include_directories(Scene PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Scene
  PUBLIC
  SerializationUserDataAdapter

  NameComponent

  Animation
  WorldRenderer

  # -- Systems:
  HierarchySystem
  AnimationSystem
  PhysicsSystem
  RenderSystem
  AudioSystem
  UISystem
  ScriptSystem
)
set_target_properties(Scene PROPERTIES FOLDER "Framework")
set_warning_level(Scene)
