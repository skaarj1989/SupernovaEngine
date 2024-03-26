define_script_module(
  TARGET Window
  SOURCES "include/LuaWindow.hpp" "src/LuaWindow.cpp"
)
add_binding("LuaWindow.hpp" "registerWindow")
