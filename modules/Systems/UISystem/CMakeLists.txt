find_package(RmlUi REQUIRED)

add_library(UISystem "include/UISystem.hpp" "src/UISystem.cpp")
target_include_directories(UISystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(UISystem
    PRIVATE CameraComponent
    PUBLIC SystemCommon UIComponent
)
set_target_properties(UISystem PROPERTIES FOLDER "Framework/Systems")
