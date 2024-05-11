#pragma once

#include "Nodes/NodeVisitor.hpp"
#include "ShaderGraphCommon.hpp"
#include "renderer/Material.hpp"
#include <unordered_set>

class ShaderGraph;

class NodeUIVisitor : public NodeVisitor {
public:
  void setContext(const ShaderGraph &, const gfx::Material::Surface *);

  void visit(EmbeddedNode<ValueVariant> &) override;
  void visit(EmbeddedNode<PropertyVariant> &) override;
  void visit(EmbeddedNode<TextureParam> &) override;

  void visit(EmbeddedNode<Attribute> &) override;
  void visit(EmbeddedNode<BuiltInConstant> &) override;
  void visit(EmbeddedNode<BuiltInSampler> &) override;
  void visit(EmbeddedNode<FrameBlockMember> &) override;
  void visit(EmbeddedNode<CameraBlockMember> &) override;

  void visit(AppendNode &) override;
  void visit(VectorSplitterNode &) override;
  void visit(MatrixSplitterNode &) override;
  void visit(SwizzleNode &) override;
  void visit(ArithmeticNode &) override;
  void visit(MatrixTransformNode &) override;
  void visit(TextureSamplingNode &) override;
  void visit(ScriptedNode &) override;
  void visit(CustomNode &) override;

  void visit(VertexMasterNode &) override;
  void visit(SurfaceMasterNode &) override;
  void visit(PostProcessMasterNode &) override;

private:
  void _addSimpleInputPin(const std::string_view, const VertexID);
  void _addSimpleOutputPin(const std::string_view, const VertexID,
                           float nodeWidth);

  enum class InputPinState : bool {
    Inactive = false,
    Active = true,
  };
  enum class InspectorMode { Inline, Popup };
  void _addInputPin(NodeBase &, const InputPinState, const InspectorMode);

private:
  const gfx::Material::Surface *m_surface{nullptr};
  std::unordered_set<VertexID> m_connectedVertices;
};
