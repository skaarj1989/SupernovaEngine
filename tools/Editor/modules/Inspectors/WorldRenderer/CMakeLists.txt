add_library(WorldRendererInspectors
  "include/PropertyInspector.hpp"
  "src/PropertyInspector.cpp"

  "include/LightInspector.hpp"
  "src/LightInspector.cpp"

  "include/MaterialInstanceInspector.hpp"
  "src/MaterialInstanceInspector.cpp"

  "include/PostProcessEffectInspector.hpp"
  "src/PostProcessEffectInspector.cpp"

  "include/SkyLightInspector.hpp"
  "src/SkyLightInspector.cpp"

  "include/RenderSettings.hpp"
  "src/RenderSettings.cpp"

  "include/WorldRendererWidget.hpp"
  "src/WorldRendererWidget.cpp"
)
target_include_directories(WorldRendererInspectors
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(WorldRendererInspectors
  PUBLIC
  WorldRenderer
  PRIVATE
  Common
  Services
  ImGuiEx
  ResourceInspector
)
set_target_properties(WorldRendererInspectors PROPERTIES
  FOLDER "Tools/EditorModules/Inspectors"
)
set_warning_level(WorldRendererInspectors)
