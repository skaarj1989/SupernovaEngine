add_executable(Cube "main.cpp")
target_link_libraries(Cube PRIVATE BaseApp VulkanRHI KTXLoader)
set_target_properties(Cube PROPERTIES FOLDER "Demo")

set_debugger_working_directory(Cube)
