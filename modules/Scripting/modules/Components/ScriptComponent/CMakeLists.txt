define_script_module(
  TARGET ScriptComponent
  SOURCES "include/LuaScriptComponent.hpp" "src/LuaScriptComponent.cpp"
)
add_binding("LuaScriptComponent.hpp" "registerScriptComponent")
