define_script_module(
  TARGET InputSystem
  SOURCES "include/LuaInputSystem.hpp" "src/LuaInputSystem.cpp"
)
add_binding("LuaInputSystem.hpp" "registerInputSystem")
