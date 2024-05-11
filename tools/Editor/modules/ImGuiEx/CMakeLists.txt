find_package(spdlog REQUIRED)

add_library(ImGuiEx
  "include/ImGuiHelper.hpp"
  "src/ImGuiHelper.cpp"
  "include/ImGuiPayloadHelper.hpp"
  "src/ImGuiPayloadHelper.cpp"
  "include/ImGuiDragAndDrop.hpp"
  "include/ImGuiPopups.hpp"
  "include/ImGuiModal.hpp"
  "src/ImGuiModal.cpp"

  "include/FileBrowser.hpp"
  "src/FileBrowser.cpp"
  "include/FileDialog.hpp"
  "src/FileDialog.cpp"

  "include/WidgetCache.hpp"
  "include/WidgetWindow.hpp"

  "include/RenderTargetPreview.hpp"
  "src/RenderTargetPreview.cpp"
  "include/TexturePreview.hpp"
  "src/TexturePreview.cpp"

  "include/EditorActionInfo.hpp"
  "src/EditorActionInfo.cpp"
  "include/TextEditorCommon.hpp"
  "src/TextEditorCommon.cpp"
)
target_include_directories(ImGuiEx
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ImGuiEx
  PUBLIC Common FileSystem InputEvents imgui::imgui
  PRIVATE spdlog::spdlog VulkanRHI
)

set_target_properties(ImGuiEx PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(ImGuiEx)

enable_profiler(ImGuiEx PRIVATE)
