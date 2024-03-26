define_script_module(
  TARGET VulkanRHI
  SOURCES "include/LuaVulkanRHI.hpp" "src/LuaVulkanRHI.cpp"
)
add_binding("LuaVulkanRHI.hpp" "registerVulkanRHI")
