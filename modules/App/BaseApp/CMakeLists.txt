find_package(spdlog REQUIRED)

add_library(BaseApp
  "include/BaseApp.hpp"
  "src/BaseApp.cpp"
  "include/RenderDoc.hpp"
  "src/RenderDoc.cpp"
)
target_include_directories(BaseApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(BaseApp PUBLIC renderdoc InputSystem VulkanRHI spdlog::spdlog)
set_target_properties(BaseApp PROPERTIES FOLDER "Framework/App")
set_warning_level(BaseApp)
