add_executable(NuklearDemo "main.cpp")
target_link_libraries(NuklearDemo PUBLIC NuklearApp)
set_target_properties(NuklearDemo PROPERTIES FOLDER "Demo")

set_debugger_working_directory(NuklearDemo)
