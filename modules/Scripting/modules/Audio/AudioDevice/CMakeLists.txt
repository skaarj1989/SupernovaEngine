define_script_module(
  TARGET AudioDevice
  SOURCES "include/LuaAudioDevice.hpp" "src/LuaAudioDevice.cpp"
)
add_binding("LuaAudioDevice.hpp" "registerAudioDevice")
