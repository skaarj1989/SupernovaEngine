define_script_module(
  TARGET Math
  SOURCES "include/LuaMath.hpp" "src/LuaMath.cpp"
)

if(MSVC)
  target_compile_options(LuaMath PRIVATE /bigobj)
endif()

add_binding("LuaMath.hpp" "registerMath")
