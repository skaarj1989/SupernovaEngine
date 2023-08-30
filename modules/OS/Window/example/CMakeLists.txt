add_executable(BasicWindow "main.cpp")
target_link_libraries(BasicWindow PRIVATE Window)
set_target_properties(BasicWindow PROPERTIES FOLDER "Framework/Test")
