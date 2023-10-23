add_library(renderdoc INTERFACE "renderdoc_app.h")
target_include_directories(renderdoc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(renderdoc PROPERTIES FOLDER "Third-party")
