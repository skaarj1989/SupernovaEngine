find_package(glm REQUIRED)

add_library(InputEvents INTERFACE "include/os/KeyCodes.hpp" "include/os/InputEvents.hpp")
target_include_directories(InputEvents INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(InputEvents INTERFACE glm::glm)
set_target_properties(InputEvents PROPERTIES FOLDER "Framework/OS")
