find_package(glm REQUIRED)

add_library(imgui
  "imconfig.h"
  "imgui_internal.h"
  "imstb_rectpack.h"
  "imstb_textedit.h"
  "imstb_truetype.h"
  "imgui_stdlib.h"
  "imgui.h"
  "imgui.cpp"
  "imgui_draw.cpp"
  "imgui_widgets.cpp"
  "imgui_tables.cpp"
  "imgui_demo.cpp"
  "imgui_stdlib.cpp"

  "TextEditor.h"
  "TextEditor.cpp"
  "LanguageDefinitions.cpp"

  "ImGuizmo.hpp"
  "ImGuizmo.cpp"

  "imnodes.h"
  "imnodes_internal.h"
  "imnodes.cpp"

  "implot.h"
  "implot.cpp"
  "implot_internal.h"
  "implot_items.cpp"
  "implot_demo.cpp"
)

target_compile_definitions(imgui
  PRIVATE IMGUI_DEFINE_MATH_OPERATORS GLM_FORCE_RADIANS
  PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS IMGUI_DISABLE_OBSOLETE_KEYIO
)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PUBLIC glm::glm)

add_library(imgui::imgui ALIAS imgui)
