find_package(nlohmann_json REQUIRED)

add_library(ImGuiStyleJSON
  "include/ImGuiStyleJSON.hpp"
  "src/ImGuiStyleJSON.cpp"
  "include/ImVecJSON.hpp"
  "src/ImVecJSON.cpp"
)
target_include_directories(ImGuiStyleJSON PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ImGuiStyleJSON
  PRIVATE nlohmann_json::nlohmann_json
  PUBLIC imgui::imgui
)
set_target_properties(ImGuiStyleJSON PROPERTIES FOLDER "Framework/GUI")
