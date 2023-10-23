define_script_module(
  TARGET WorldRenderer
  SOURCES "include/LuaWorldRenderer.hpp" "src/LuaWorldRenderer.cpp"
)

if(MSVC)
  target_compile_options(LuaWorldRenderer PRIVATE /bigobj)
endif()
