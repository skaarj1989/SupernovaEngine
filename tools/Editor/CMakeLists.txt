enable_testing()
add_subdirectory(tests)

find_package(cereal CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS graph)

set(WIDGET_SOURCES
  "include/FileBrowser.hpp"
  "src/FileBrowser.cpp"
  "include/FileDialog.hpp"
  "src/FileDialog.cpp"

  "include/ProjectSettingsWidget.hpp"
  "src/ProjectSettingsWidget.cpp"

  "include/GPUWidget.hpp"
  "src/GPUWidget.cpp"
  "include/RenderSettings.hpp"
  "src/RenderSettings.cpp"
  "include/RenderTargetPreview.hpp"

  "include/ShapeCreatorWidget.hpp"
  "src/ShapeCreatorWidget.cpp"

  "include/WidgetWindow.hpp"

  "src/RenderTargetPreview.cpp"
  "include/WorldRendererWidget.hpp"
  "src/WorldRendererWidget.cpp"
  "include/LoggerWidget.hpp"
  "src/LoggerWidget.cpp"

  "include/WidgetCache.hpp"

  "include/ResourceDockSpace.hpp"
  "src/ResourceDockSpace.cpp"
  "src/CacheInspector.hpp"
  "include/SkeletonCache.hpp"
  "src/SkeletonCache.cpp"
  "include/AnimationCache.hpp"
  "src/AnimationCache.cpp"
  "include/ColliderCache.hpp"
  "src/ColliderCache.cpp"
  "include/MeshCache.hpp"
  "src/MeshCache.cpp"
  "include/MaterialCache.hpp"
  "src/MaterialCache.cpp"
  "include/TextureCache.hpp"
  "src/TextureCache.cpp"
  "include/AudioClipCache.hpp"
  "src/AudioClipCache.cpp"
  "include/ScriptCache.hpp"
  "src/ScriptCache.cpp"

  "include/Inspectors/TransformInspector.hpp"
  "src/Inspectors/TransformInspector.cpp"

  "include/Inspectors/ResourceInspector.hpp"
  "src/Inspectors/ResourceInspector.cpp"

  "include/Inspectors/AnimationInspector.hpp"
  "src/Inspectors/AnimationInspector.cpp"
  "include/Inspectors/SkeletonInspector.hpp"
  "src/Inspectors/SkeletonInspector.cpp"

  "include/Inspectors/CollisionLayerInspector.hpp"
  "src/Inspectors/CollisionLayerInspector.cpp"
  "include/Inspectors/ColliderInspector.hpp"
  "src/Inspectors/ColliderInspector.cpp"
  "include/Inspectors/RigidBodyInspector.hpp"
  "src/Inspectors/RigidBodyInspector.cpp"
  "include/Inspectors/CharacterInspector.hpp"
  "src/Inspectors/CharacterInspector.cpp"
  "include/Inspectors/CharacterVirtualInspector.hpp"
  "src/Inspectors/CharacterVirtualInspector.cpp"

  "include/Inspectors/CameraInspector.hpp"
  "src/Inspectors/CameraInspector.cpp"

  "include/Inspectors/LightInspector.hpp"
  "src/Inspectors/LightInspector.cpp"
  "include/Inspectors/PropertyInspector.hpp"
  "src/Inspectors/PropertyInspector.cpp"
  "include/Inspectors/MaterialInstanceInspector.hpp"
  "src/Inspectors/MaterialInstanceInspector.cpp"
  "include/Inspectors/PostProcessEffectInspector.hpp"
  "src/Inspectors/PostProcessEffectInspector.cpp"
  "include/Inspectors/SkyLightInspector.hpp"
  "src/Inspectors/SkyLightInspector.cpp"

  "include/TexturePreview.hpp"
  "src/TexturePreview.cpp"
)
set(MATERIAL_EDITOR_SOURCES
  "include/MaterialEditor/MaterialEditor.hpp"
  "src/MaterialEditor/MaterialEditor.cpp"
  "include/MaterialEditor/MaterialProject.hpp"
  "src/MaterialEditor/MaterialProject.cpp"

  "include/MaterialEditor/NodeEditor.hpp"
  "src/MaterialEditor/NodeEditor.cpp"

  "include/MaterialEditor/Command.hpp"
  "include/MaterialEditor/CommandInvoker.hpp"
  "include/MaterialEditor/NodeFactoryRegistry.hpp"
  "src/MaterialEditor/NodeFactoryRegistry.cpp"
  "include/MaterialEditor/PathMap.hpp"
  "src/MaterialEditor/PathMap.cpp"
  "include/MaterialEditor/NodePatcherVisitor.hpp"
  "src/MaterialEditor/NodePatcherVisitor.cpp"

  "include/MaterialEditor/ShaderCodeEvaluator.hpp"
  "src/MaterialEditor/ShaderCodeEvaluator.cpp"
  "src/MaterialEditor/Evaluator/Arithmetic.cpp"
  "src/MaterialEditor/Evaluator/BuiltIns.cpp"
  "src/MaterialEditor/Evaluator/Master.cpp"
  "include/MaterialEditor/ShaderCodeComposer.hpp"
  "src/MaterialEditor/ShaderCodeComposer.cpp"

  "include/MaterialEditor/NodeUIVisitor.hpp"
  "src/MaterialEditor/NodeUIVisitor.cpp"

  "include/MaterialEditor/CustomNodeEditor.hpp"
  "src/MaterialEditor/CustomNodeEditor.cpp"
  "include/MaterialEditor/NodeContextMenuEntries.hpp"
  "src/MaterialEditor/NodeContextMenuEntries.cpp"
  "include/MaterialEditor/MaterialPreviewWidget.hpp"
  "src/MaterialEditor/MaterialPreviewWidget.cpp"

  # ---
  "include/MaterialEditor/ValueVariant.hpp"
  "src/MaterialEditor/ValueVariant.cpp"
  "include/MaterialEditor/PropertyVariant.hpp"
  "src/MaterialEditor/PropertyVariant.cpp"
  "include/MaterialEditor/TextureParam.hpp"
  "src/MaterialEditor/TextureParam.cpp"
  "include/MaterialEditor/TransientVariant.hpp"
  "src/MaterialEditor/TransientVariant.cpp"

  "include/MaterialEditor/Attribute.hpp"
  "src/MaterialEditor/Attribute.cpp"
  "include/MaterialEditor/BuiltInConstants.hpp"
  "src/MaterialEditor/BuiltInConstants.cpp"
  "include/MaterialEditor/CameraBlockMember.hpp"
  "src/MaterialEditor/CameraBlockMember.cpp"
  "include/MaterialEditor/FrameBlockMember.hpp"
  "src/MaterialEditor/FrameBlockMember.cpp"
  "include/MaterialEditor/SplitMatrix.hpp"
  "include/MaterialEditor/SplitVector.hpp"

  "include/MaterialEditor/Nodes/fwd.hpp"
  "include/MaterialEditor/Nodes/NodeVisitor.hpp"
  "include/MaterialEditor/Nodes/NodeBase.hpp"
  "src/MaterialEditor/Nodes/NodeBase.cpp"
  "include/MaterialEditor/Nodes/Empty.hpp"
  "include/MaterialEditor/Nodes/Embedded.hpp"
  "include/MaterialEditor/Nodes/Compound.hpp"
  "src/MaterialEditor/Nodes/Compound.cpp"
  "include/MaterialEditor/Nodes/Append.hpp"
  "src/MaterialEditor/Nodes/Append.cpp"
  "include/MaterialEditor/Nodes/VectorSplitter.hpp"
  "src/MaterialEditor/Nodes/VectorSplitter.cpp"
  "include/MaterialEditor/Nodes/MatrixSplitter.hpp"
  "src/MaterialEditor/Nodes/MatrixSplitter.cpp"
  "include/MaterialEditor/Nodes/Swizzle.hpp"
  "src/MaterialEditor/Nodes/Swizzle.cpp"
  "include/MaterialEditor/Nodes/Arithmetic.hpp"
  "src/MaterialEditor/Nodes/Arithmetic.cpp"
  "include/MaterialEditor/Nodes/MatrixTransform.hpp"
  "src/MaterialEditor/Nodes/MatrixTransform.cpp"
  "include/MaterialEditor/Nodes/TextureSampling.hpp"
  "src/MaterialEditor/Nodes/TextureSampling.cpp"
  "include/MaterialEditor/Nodes/Scripted.hpp"
  "src/MaterialEditor/Nodes/Scripted.cpp"
  "include/MaterialEditor/Nodes/Custom.hpp"
  "src/MaterialEditor/Nodes/Custom.cpp"

  "include/MaterialEditor/Nodes/VertexMaster.hpp"
  "src/MaterialEditor/Nodes/VertexMaster.cpp"
  "include/MaterialEditor/Nodes/SurfaceMaster.hpp"
  "src/MaterialEditor/Nodes/SurfaceMaster.cpp"
  "include/MaterialEditor/Nodes/PostProcessMaster.hpp"
  "src/MaterialEditor/Nodes/PostProcessMaster.cpp"

  # ---
  "include/MaterialEditor/ScriptedFunction.hpp"
  "include/MaterialEditor/UserFunction.hpp"
  "src/MaterialEditor/UserFunction.cpp"

  "include/MaterialEditor/ShaderGraph.hpp"
  "src/MaterialEditor/ShaderGraph.cpp"
  "include/MaterialEditor/EdgeProperty.hpp"
  "include/MaterialEditor/ShaderGraphCommon.hpp"
  "include/MaterialEditor/ShaderGraphStageView.hpp"

  "include/MaterialEditor/DataType.hpp"
  "src/MaterialEditor/DataType.cpp"

  "include/MaterialEditor/ShaderToken.hpp"

  # ---
  "src/MaterialEditor/Nodes/Nodes.cpp"
  "src/MaterialEditor/Nodes/Utility.hpp"
  "src/MaterialEditor/Nodes/Utility.cpp"
  "src/MaterialEditor/TraverseGraph.hpp"
  "src/MaterialEditor/FunctionAvailability.hpp"
  "src/MaterialEditor/FunctionAvailability.cpp"
  "src/MaterialEditor/GraphCommands.hpp"
  "src/MaterialEditor/GraphCommands.cpp"
  "src/MaterialEditor/ProjectCommands.hpp"
  "src/MaterialEditor/ProjectCommands.cpp"
  "src/MaterialEditor/MaterialExporter.hpp"
  "src/MaterialEditor/MaterialExporter.cpp"
  "src/MaterialEditor/LuaMaterialEditor.hpp"
  "src/MaterialEditor/LuaMaterialEditor.cpp"

  "src/MaterialEditor/ImNodesHelper.hpp"
  "src/MaterialEditor/ImNodesHelper.cpp"
  "src/MaterialEditor/ImNodesStyleWidget.hpp"
  "src/MaterialEditor/ImNodesStyleWidget.cpp"
  "src/MaterialEditor/ImNodesStyleJSON.hpp"
  "src/MaterialEditor/ImNodesStyleJSON.cpp"

  "src/MaterialEditor/ChangeEnumCombo.hpp"
  "src/MaterialEditor/ChangeVariantCombo.hpp"
)
set(SCENE_EDITOR_SOURCES
  "include/Inspector.hpp"

  "include/SceneEditor/SceneEditor.hpp"
  "src/SceneEditor/SceneEditor.cpp"
  "include/SceneEditor/GizmoController.hpp"
  "src/SceneEditor/GizmoController.cpp"

  "src/SceneEditor/Inspectors/NameComponent.cpp"
  "src/SceneEditor/Inspectors/Transform.cpp"
  "src/SceneEditor/Inspectors/Hierarchy.cpp"
  "src/SceneEditor/Inspectors/Animation/SkeletonComponent.cpp"
  "src/SceneEditor/Inspectors/Animation/AnimationComponent.cpp"
  "src/SceneEditor/Inspectors/Animation/PlaybackController.cpp"
  "src/SceneEditor/Inspectors/Physics/Character.cpp"
  "src/SceneEditor/Inspectors/Physics/Collider.cpp"
  "src/SceneEditor/Inspectors/Physics/RigidBody.cpp"
  "src/SceneEditor/Inspectors/Renderer/Camera.cpp"
  "src/SceneEditor/Inspectors/Renderer/Light.cpp"
  "src/SceneEditor/Inspectors/Renderer/Mesh.cpp"
  "src/SceneEditor/Inspectors/Audio/ListenerComponentInspector.cpp"
  "src/SceneEditor/Inspectors/Audio/AudioComponentInspector.hpp"
  "src/SceneEditor/Inspectors/Audio/SoundSourceComponent.cpp"
  "src/SceneEditor/Inspectors/Audio/AudioPlayerComponent.cpp"
  "src/SceneEditor/Inspectors/Script.cpp"
)
set(SOURCES
  "src/main.cpp"

  "include/App.hpp"
  "src/App.cpp"
  "include/Services.hpp"
  "src/Services.cpp"

  "include/ProjectSettings.hpp"
  "src/ProjectSettings.cpp"

  "include/ImGuiHelper.hpp"
  "src/ImGuiHelper.cpp"
  "include/ImGuiPayloadType.hpp"
  "include/ImGuiPayloadHelper.hpp"
  "src/ImGuiPayloadHelper.cpp"
  "include/ImGuiDragAndDrop.hpp"
  "include/ImGuiPopups.hpp"
  "include/ImGuiModal.hpp"
  "src/ImGuiModal.cpp"

  "include/CameraController.hpp"
  "src/CameraController.cpp"

  "include/EditorActionInfo.hpp"
  "src/EditorActionInfo.cpp"
  "include/TextEditorCommon.hpp"
  "src/TextEditorCommon.cpp"

  "include/ScriptEditor.hpp"
  "src/ScriptEditor.cpp"

  ${WIDGET_SOURCES}
  ${MATERIAL_EDITOR_SOURCES}
  ${SCENE_EDITOR_SOURCES}
)

add_executable(Editor ${SOURCES})
target_include_directories(Editor
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(Editor
  PRIVATE
  ImGuiApp
  ImGuiStyleJSON
  StringUtility
  SerializationUserDataAdapter
  MetaComponent
  MetaHelper
  Scene

  RmlUiPlatformInterface
  RmlUiRenderer

  Boost::graph

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
group_msvc_project(TARGET Editor)
set_debugger_working_directory(Editor)

add_resources(
  TARGET EditorAssets
  IN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets
  OUT_DIR assets
)
add_dependencies(Editor Copy-EditorAssets)
