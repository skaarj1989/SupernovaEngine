define_script_module(
  TARGET DebugDraw
  SOURCES "include/LuaDebugDraw.hpp" "src/LuaDebugDraw.cpp"
)
add_binding("LuaDebugDraw.hpp" "registerDebugDraw")
