add_executable(HelloTriangle "main.cpp")
target_link_libraries(HelloTriangle PRIVATE VulkanRHI spdlog::spdlog)
set_target_properties(HelloTriangle PROPERTIES FOLDER "Framework/RHI")
set_warning_level(HelloTriangle)
