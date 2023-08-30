define_script_module(
  TARGET WorldRenderer
  SOURCES "include/LuaWorldRenderer.hpp" "src/LuaWorldRenderer.cpp"
)

if(MSVC)
  target_compile_options(LuaWorldRenderer PRIVATE /bigobj)
else()
  target_compile_options(LuaWorldRenderer PRIVATE -Wa,-mbig-obj)
endif()
