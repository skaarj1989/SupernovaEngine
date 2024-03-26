define_script_module(
  TARGET Animation
  SOURCES "include/LuaAnimation.hpp" "src/LuaAnimation.cpp"
)
add_binding("LuaAnimation.hpp" "registerAnimation")
