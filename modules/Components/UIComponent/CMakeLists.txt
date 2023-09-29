add_library(UIComponent INTERFACE "UIComponent.hpp")
target_include_directories(UIComponent INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(UIComponent INTERFACE RmlUiRenderer)
set_target_properties(UIComponent PROPERTIES FOLDER "Framework/Components")
