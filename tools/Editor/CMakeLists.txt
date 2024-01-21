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

  "include/Inspectors/JoltInspector.hpp"
  "src/Inspectors/JoltInspector.cpp"
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

  # -- Public header files:
  "include/MaterialEditor/ChangeEnumCombo.hpp"
  "include/MaterialEditor/ChangeVariantCombo.hpp"
  "include/MaterialEditor/FunctionAvailability.hpp"

  "include/MaterialEditor/Attribute.hpp"
  "include/MaterialEditor/FrameBlockMember.hpp"
  "include/MaterialEditor/CameraBlockMember.hpp"
  "include/MaterialEditor/BuiltInConstants.hpp"
  "include/MaterialEditor/ShaderToken.hpp"
  "include/MaterialEditor/MaterialGenerationContext.hpp"
  "include/MaterialEditor/DataType.hpp"
  "include/MaterialEditor/ScriptedFunctionData.hpp"
  "include/MaterialEditor/ImNodesHelper.hpp"
  "include/MaterialEditor/LuaMaterialEditor.hpp"
  "include/MaterialEditor/MaterialEditor.hpp"
  "include/MaterialEditor/IDPair.hpp"
  "include/MaterialEditor/Connection.hpp"
  "include/MaterialEditor/ShaderGraph.hpp"
  "include/MaterialEditor/VertexDescriptorSerializer.hpp"
  "include/MaterialEditor/ShaderGraphCommon.hpp"
  "include/MaterialEditor/MaterialPreviewWidget.hpp"
  "include/MaterialEditor/Property.hpp"
  "include/MaterialEditor/ShaderCodeComposer.hpp"
  "include/MaterialEditor/SplitVector.hpp"
  "include/MaterialEditor/SplitMatrix.hpp"
  "include/MaterialEditor/TextureParam.hpp"
  "include/MaterialEditor/ValueVariant.hpp"

  "include/MaterialEditor/CompoundNodeVariant.hpp"
  "include/MaterialEditor/MasterVariant.hpp"
  "include/MaterialEditor/VertexProp.hpp"
  "include/MaterialEditor/EdgeProp.hpp"

  "include/MaterialEditor/Nodes/FrameBlock.hpp"
  "include/MaterialEditor/Nodes/CameraBlock.hpp"
  "include/MaterialEditor/Nodes/Container.hpp"

  "include/MaterialEditor/UserFunctionData.hpp"
  "include/MaterialEditor/CustomNodeEditor.hpp"

  "include/MaterialEditor/Nodes/Append.hpp"
  "include/MaterialEditor/Nodes/NodeCommon.hpp"
  "include/MaterialEditor/Nodes/Arithmetic.hpp"
  "include/MaterialEditor/Nodes/Scripted.hpp"
  "include/MaterialEditor/Nodes/Custom.hpp"
  "include/MaterialEditor/Nodes/MatrixTransform.hpp"
  "include/MaterialEditor/Nodes/VectorSplitter.hpp"
  "include/MaterialEditor/Nodes/MatrixSplitter.hpp"
  "include/MaterialEditor/Nodes/Swizzle.hpp"
  "include/MaterialEditor/Nodes/TextureSampling.hpp"
  "include/MaterialEditor/Nodes/VertexMaster.hpp"
  "include/MaterialEditor/Nodes/SurfaceMaster.hpp"
  "include/MaterialEditor/Nodes/PostProcessMaster.hpp"

  # -- Source (and private headers) files:
  "src/MaterialEditor/ImNodesStyleWidget.hpp"
  "src/MaterialEditor/ImNodesStyleWidget.cpp"
  "src/MaterialEditor/ImNodesStyleJSON.hpp"
  "src/MaterialEditor/ImNodesStyleJSON.cpp"

  "src/MaterialEditor/FunctionAvailability.cpp"

  "src/MaterialEditor/ShaderInfoLogParser.hpp"
  "src/MaterialEditor/ShaderInfoLogParser.cpp"

  "src/MaterialEditor/InspectEnum.hpp"

  "src/MaterialEditor/UserFunctionData.cpp"
  "src/MaterialEditor/CustomNodeEditor.cpp"

  "src/MaterialEditor/CreateNodeHelper.hpp"

  "src/MaterialEditor/NodeContextMenuEntries.hpp"
  "src/MaterialEditor/NodeContextMenuEntries.cpp"

  "src/MaterialEditor/Attribute.cpp"
  "src/MaterialEditor/FrameBlockMember.cpp"
  "src/MaterialEditor/CameraBlockMember.cpp"
  "src/MaterialEditor/BuiltInConstants.cpp"
  "src/MaterialEditor/MaterialGenerationContext.cpp"
  "src/MaterialEditor/DataType.cpp"
  "src/MaterialEditor/ImNodesHelper.cpp"
  "src/MaterialEditor/LuaMaterialEditor.cpp"
  "src/MaterialEditor/MaterialEditor.cpp"
  "src/MaterialEditor/CycleDetector.hpp"
  "src/MaterialEditor/TraverseGraph.hpp"
  "src/MaterialEditor/ShaderGraph.cpp"
  "src/MaterialEditor/MaterialPreviewWidget.cpp"
  "src/MaterialEditor/Property.cpp"
  "src/MaterialEditor/ShaderCodeComposer.cpp"
  "src/MaterialEditor/SplitVector.cpp"
  "src/MaterialEditor/SplitMatrix.cpp"
  "src/MaterialEditor/TextureParam.cpp"
  "src/MaterialEditor/ValueVariant.cpp"

  "src/MaterialEditor/CompoundNodeVariant.cpp"
  "src/MaterialEditor/MasterVariant.cpp"
  "src/MaterialEditor/VertexProp.cpp"

  "include/MaterialEditor/PathMap.hpp"
  "src/MaterialEditor/PathMap.cpp"

  "include/MaterialEditor/MaterialProject.hpp"
  "src/MaterialEditor/MaterialProject.cpp"

  "src/MaterialEditor/MaterialExporter.hpp"
  "src/MaterialEditor/MaterialExporter.cpp"

  "src/MaterialEditor/Nodes/NodesInternal.hpp"
  "src/MaterialEditor/Nodes/NodesInternal.cpp"

  "src/MaterialEditor/Nodes/NodeCommon.cpp"

  "src/MaterialEditor/Nodes/FrameBlock.cpp"
  "src/MaterialEditor/Nodes/CameraBlock.cpp"
  "src/MaterialEditor/Nodes/Container.cpp"

  "src/MaterialEditor/Nodes/Append.cpp"
  "src/MaterialEditor/Nodes/Arithmetic.cpp"
  "src/MaterialEditor/Nodes/Scripted.cpp"
  "src/MaterialEditor/Nodes/Custom.cpp"
  "src/MaterialEditor/Nodes/MatrixTransform.cpp"
  "src/MaterialEditor/Nodes/PostProcessMaster.cpp"
  "src/MaterialEditor/Nodes/VectorSplitter.cpp"
  "src/MaterialEditor/Nodes/MatrixSplitter.cpp"
  "src/MaterialEditor/Nodes/SurfaceMaster.cpp"
  "src/MaterialEditor/Nodes/Swizzle.cpp"
  "src/MaterialEditor/Nodes/TextureSampling.cpp"
  "src/MaterialEditor/Nodes/VertexMaster.cpp"
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

  "include/ScriptEditor.hpp"
  "src/ScriptEditor.cpp"

  ${WIDGET_SOURCES}
  ${MATERIAL_EDITOR_SOURCES}
  ${SCENE_EDITOR_SOURCES}
)

add_executable(Editor ${SOURCES})
target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
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
