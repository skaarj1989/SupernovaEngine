define_script_module(
  TARGET Resource
  SOURCES "include/LuaResource.hpp" "src/LuaResource.cpp"
)
add_binding("LuaResource.hpp" "registerResource")
