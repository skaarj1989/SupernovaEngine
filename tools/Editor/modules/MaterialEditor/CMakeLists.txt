find_package(spdlog REQUIRED)
find_package(Boost REQUIRED COMPONENTS graph)
find_package(cereal CONFIG REQUIRED)

set(MATERIAL_EDITOR_SOURCES
  "include/MaterialEditor.hpp"
  "src/MaterialEditor.cpp"
  "include/MaterialProject.hpp"
  "src/MaterialProject.cpp"

  "include/NodeEditor.hpp"
  "src/NodeEditor.cpp"

  "include/Command.hpp"
  "include/CommandInvoker.hpp"
  "include/NodeFactoryRegistry.hpp"
  "src/NodeFactoryRegistry.cpp"
  "include/PathMap.hpp"
  "src/PathMap.cpp"
  "include/NodePatcherVisitor.hpp"
  "src/NodePatcherVisitor.cpp"

  "include/ShaderCodeEvaluator.hpp"
  "src/ShaderCodeEvaluator.cpp"
  "src/Evaluator/Arithmetic.cpp"
  "src/Evaluator/BuiltIns.cpp"
  "src/Evaluator/Master.cpp"
  "include/ShaderCodeComposer.hpp"
  "src/ShaderCodeComposer.cpp"

  "include/NodeUIVisitor.hpp"
  "src/NodeUIVisitor.cpp"

  "include/CustomNodeEditor.hpp"
  "src/CustomNodeEditor.cpp"
  "include/NodeContextMenuEntries.hpp"
  "src/NodeContextMenuEntries.cpp"
  "include/MaterialPreviewWidget.hpp"
  "src/MaterialPreviewWidget.cpp"

  "include/ValueVariant.hpp"
  "src/ValueVariant.cpp"
  "include/PropertyVariant.hpp"
  "src/PropertyVariant.cpp"
  "include/TextureParam.hpp"
  "src/TextureParam.cpp"
  "include/TransientVariant.hpp"
  "src/TransientVariant.cpp"

  "include/Attribute.hpp"
  "src/Attribute.cpp"
  "include/BuiltInConstants.hpp"
  "src/BuiltInConstants.cpp"
  "include/CameraBlockMember.hpp"
  "src/CameraBlockMember.cpp"
  "include/FrameBlockMember.hpp"
  "src/FrameBlockMember.cpp"
  "include/SplitMatrix.hpp"
  "include/SplitVector.hpp"

  "include/Nodes/fwd.hpp"
  "include/Nodes/NodeVisitor.hpp"
  "include/Nodes/NodeBase.hpp"
  "src/Nodes/NodeBase.cpp"
  "include/Nodes/Empty.hpp"
  "include/Nodes/Embedded.hpp"
  "include/Nodes/Compound.hpp"
  "src/Nodes/Compound.cpp"
  "include/Nodes/Append.hpp"
  "src/Nodes/Append.cpp"
  "include/Nodes/VectorSplitter.hpp"
  "src/Nodes/VectorSplitter.cpp"
  "include/Nodes/MatrixSplitter.hpp"
  "src/Nodes/MatrixSplitter.cpp"
  "include/Nodes/Swizzle.hpp"
  "src/Nodes/Swizzle.cpp"
  "include/Nodes/Arithmetic.hpp"
  "src/Nodes/Arithmetic.cpp"
  "include/Nodes/MatrixTransform.hpp"
  "src/Nodes/MatrixTransform.cpp"
  "include/Nodes/TextureSampling.hpp"
  "src/Nodes/TextureSampling.cpp"
  "include/Nodes/Scripted.hpp"
  "src/Nodes/Scripted.cpp"
  "include/Nodes/Custom.hpp"
  "src/Nodes/Custom.cpp"

  "include/Nodes/VertexMaster.hpp"
  "src/Nodes/VertexMaster.cpp"
  "include/Nodes/SurfaceMaster.hpp"
  "src/Nodes/SurfaceMaster.cpp"
  "include/Nodes/PostProcessMaster.hpp"
  "src/Nodes/PostProcessMaster.cpp"

  "include/ScriptedFunction.hpp"
  "include/UserFunction.hpp"
  "src/UserFunction.cpp"

  "include/ShaderGraph.hpp"
  "src/ShaderGraph.cpp"
  "include/EdgeProperty.hpp"
  "include/ShaderGraphCommon.hpp"
  "include/ShaderGraphStageView.hpp"

  "include/DataType.hpp"
  "src/DataType.cpp"

  "include/ShaderToken.hpp"

  "src/Nodes/Nodes.cpp"
  "src/Nodes/Utility.hpp"
  "src/Nodes/Utility.cpp"
  "src/TraverseGraph.hpp"
  "src/FunctionAvailability.hpp"
  "src/FunctionAvailability.cpp"
  "src/GraphCommands.hpp"
  "src/GraphCommands.cpp"
  "src/ProjectCommands.hpp"
  "src/ProjectCommands.cpp"
  "src/MaterialExporter.hpp"
  "src/MaterialExporter.cpp"
  "src/LuaMaterialEditor.hpp"
  "src/LuaMaterialEditor.cpp"

  "src/ImNodesHelper.hpp"
  "src/ImNodesHelper.cpp"
  "src/ImNodesStyleWidget.hpp"
  "src/ImNodesStyleWidget.cpp"
  "src/ImNodesStyleJSON.hpp"
  "src/ImNodesStyleJSON.cpp"

  "src/ChangeEnumCombo.hpp"
  "src/ChangeVariantCombo.hpp"
)

add_library(MaterialEditor ${MATERIAL_EDITOR_SOURCES})
target_include_directories(MaterialEditor
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(MaterialEditor
  PUBLIC
  Boost::graph
  cereal::cereal

  WorldRenderer

  CameraController
  GizmoController

  PRIVATE
  spdlog::spdlog
  Services

  InputSystem

  ImGuiEx
  TransformInspector
  WorldRendererInspectors

  ImGuiStyleJSON

  LuaMath
)
set_target_properties(MaterialEditor PROPERTIES FOLDER "Tools/EditorModules")
set_warning_level(MaterialEditor)

group_msvc_project(TARGET MaterialEditor)
enable_profiler(MaterialEditor PUBLIC)

enable_testing()
add_subdirectory(tests)
