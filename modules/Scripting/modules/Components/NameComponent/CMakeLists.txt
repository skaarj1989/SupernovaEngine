define_script_module(
  TARGET NameComponent
  SOURCES "include/LuaNameComponent.hpp" "src/LuaNameComponent.cpp"
)
add_binding("LuaNameComponent.hpp" "registerNameComponent")
