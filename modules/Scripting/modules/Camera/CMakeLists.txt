define_script_module(
  TARGET Camera
  SOURCES "include/LuaCamera.hpp" "src/LuaCamera.cpp"
)
add_binding("LuaCamera.hpp" "registerCamera")
