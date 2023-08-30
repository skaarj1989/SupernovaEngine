define_script_module(
  TARGET JoltPhysics
  SOURCES "include/LuaPhysics.hpp" "src/LuaPhysics.cpp"
  LIBRARIES LuaEmitter
)