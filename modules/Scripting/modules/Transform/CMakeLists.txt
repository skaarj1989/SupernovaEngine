define_script_module(
  TARGET Transform
  SOURCES "include/LuaTransform.hpp" "src/LuaTransform.cpp"
)
add_binding("LuaTransform.hpp" "registerTransform")
