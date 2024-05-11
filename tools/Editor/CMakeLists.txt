add_subdirectory(modules)

set(SOURCES
  "src/main.cpp"

  "include/App.hpp"
  "src/App.cpp"

  "include/ProjectSettings.hpp"
  "src/ProjectSettings.cpp"

  "include/ProjectSettingsWidget.hpp"
  "src/ProjectSettingsWidget.cpp"

  "include/GPUWidget.hpp"
  "src/GPUWidget.cpp"

  "include/ShapeCreatorWidget.hpp"
  "src/ShapeCreatorWidget.cpp"

  "include/LoggerWidget.hpp"
  "src/LoggerWidget.cpp"
)

add_executable(Editor ${SOURCES})
target_include_directories(Editor
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(Editor PRIVATE
  Services
  ImGuiApp
  ImGuiStyleJSON
  StringUtility
  MetaComponent
  MetaHelper
  Scene

  RmlUiPlatformInterface
  RmlUiRenderer

  ImGuiEx

  TransformInspector
  WorldRendererInspectors
  PhysicsInspectors

  ResourceDockSpace
  SceneEditor
  MaterialEditor
  ScriptEditor

  # -- Scripting:
  LuaUtility
  LuaLogger
  LuaMath
  LuaFileSystem
  LuaResource
  LuaEntity
  LuaDispatcher
  LuaScheduler
  LuaTransform
  LuaWindow
  LuaInputSystem
  LuaDebugDraw
  LuaAnimation

  LuaJoltPhysics
  LuaCamera
  LuaVulkanRHI
  LuaWorldRenderer
  LuaCameraComponent
  LuaUIComponent
  LuaDebugDraw
  LuaRmlUi
  LuaAudioDevice
  LuaAudioWorld
  LuaNameComponent
  LuaScriptComponent
)

if(MSVC)
  target_compile_options(Editor PRIVATE /bigobj)
endif()

set_target_properties(Editor PROPERTIES
  FOLDER "Tools"
  VS_DEBUGGER_COMMAND_ARGUMENTS "../../projects/Samples/Samples.project"
)
set_warning_level(Editor)

group_msvc_project(TARGET Editor)
set_debugger_working_directory(Editor)

add_resources(
  TARGET EditorAssets
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets
  OUT_DIR assets
)
add_dependencies(Editor Copy-EditorAssets)
