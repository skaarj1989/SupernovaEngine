define_script_module(
  TARGET UIComponent
  SOURCES "include/LuaUIComponent.hpp" "src/LuaUIComponent.cpp"
)
add_binding("LuaUIComponent.hpp" "registerUIComponent")
