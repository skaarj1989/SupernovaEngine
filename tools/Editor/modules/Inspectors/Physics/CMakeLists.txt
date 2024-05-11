add_library(PhysicsInspectors
  "include/CharacterInspector.hpp"
  "src/CharacterInspector.cpp"

  "include/CharacterVirtualInspector.hpp"
  "src/CharacterVirtualInspector.cpp"

  "include/ColliderInspector.hpp"
  "src/ColliderInspector.cpp"

  "include/CollisionLayerInspector.hpp"
  "src/CollisionLayerInspector.cpp"

  "include/RigidBodyInspector.hpp"
  "src/RigidBodyInspector.cpp"
)
target_include_directories(PhysicsInspectors
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(PhysicsInspectors PUBLIC JoltPhysics PRIVATE ImGuiEx)
set_target_properties(PhysicsInspectors PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(PhysicsInspectors)
