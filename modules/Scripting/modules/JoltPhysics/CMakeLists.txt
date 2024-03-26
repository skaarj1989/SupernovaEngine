define_script_module(
  TARGET JoltPhysics
  SOURCES "include/LuaPhysics.hpp" "src/LuaPhysics.cpp"
  LIBRARIES LuaEmitter
)

if(MSVC)
  target_compile_options(LuaJoltPhysics PRIVATE /bigobj)
endif()

add_binding("LuaPhysics.hpp" "registerJoltPhysics")
