define_script_module(
  TARGET Math
  SOURCES "include/LuaMath.hpp" "src/LuaMath.cpp"
)

if(MSVC)
  target_compile_options(LuaMath PRIVATE /bigobj)
else()
  target_compile_options(LuaMath PRIVATE -Wa,-mbig-obj)
endif()
