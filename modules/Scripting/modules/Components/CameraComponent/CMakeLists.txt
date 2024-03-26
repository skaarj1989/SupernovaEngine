define_script_module(
  TARGET CameraComponent
  SOURCES "include/LuaCameraComponent.hpp" "src/LuaCameraComponent.cpp"
)
add_binding("LuaCameraComponent.hpp" "registerCameraComponent")
