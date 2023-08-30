add_executable(ImGuiDemo "main.cpp")
target_link_libraries(ImGuiDemo PUBLIC ImGuiApp)
set_target_properties(ImGuiDemo PROPERTIES FOLDER "Demo")

set_debugger_working_directory(ImGuiDemo)
