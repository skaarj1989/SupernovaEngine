define_script_module(
  TARGET AudioWorld
  SOURCES "include/LuaAudioWorld.hpp" "src/LuaAudioWorld.cpp"
)
add_binding("LuaAudioWorld.hpp" "registerAudioWorld")
