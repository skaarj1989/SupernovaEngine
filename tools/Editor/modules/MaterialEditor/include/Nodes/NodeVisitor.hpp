#pragma once

#include "ValueVariant.hpp"
#include "PropertyVariant.hpp"
#include "TextureParam.hpp"
#include "fwd.hpp"

class NodeVisitor {
public:
  virtual void visit(EmptyNode &) {}
  virtual void visit(const EmptyNode &) {}

  virtual void visit(EmbeddedNode<ValueVariant> &) {}
  virtual void visit(const EmbeddedNode<ValueVariant> &) {}
  virtual void visit(EmbeddedNode<PropertyVariant> &) {}
  virtual void visit(const EmbeddedNode<PropertyVariant> &) {}
  virtual void visit(EmbeddedNode<TextureParam> &) {}
  virtual void visit(const EmbeddedNode<TextureParam> &) {}

  virtual void visit(EmbeddedNode<Attribute> &) {}
  virtual void visit(const EmbeddedNode<Attribute> &) {}
  virtual void visit(EmbeddedNode<BuiltInConstant> &) {}
  virtual void visit(const EmbeddedNode<BuiltInConstant> &) {}
  virtual void visit(EmbeddedNode<BuiltInSampler> &) {}
  virtual void visit(const EmbeddedNode<BuiltInSampler> &) {}
  virtual void visit(EmbeddedNode<FrameBlockMember> &) {}
  virtual void visit(const EmbeddedNode<FrameBlockMember> &) {}
  virtual void visit(EmbeddedNode<CameraBlockMember> &) {}
  virtual void visit(const EmbeddedNode<CameraBlockMember> &) {}

  virtual void visit(EmbeddedNode<SplitVector> &) {}
  virtual void visit(const EmbeddedNode<SplitVector> &) {}
  virtual void visit(EmbeddedNode<SplitMatrix> &) {}
  virtual void visit(const EmbeddedNode<SplitMatrix> &) {}

  virtual void visit(AppendNode &) {}
  virtual void visit(const AppendNode &) {}
  virtual void visit(VectorSplitterNode &) {}
  virtual void visit(const VectorSplitterNode &) {}
  virtual void visit(MatrixSplitterNode &) {}
  virtual void visit(const MatrixSplitterNode &) {}
  virtual void visit(SwizzleNode &) {}
  virtual void visit(const SwizzleNode &) {}
  virtual void visit(ArithmeticNode &) {}
  virtual void visit(const ArithmeticNode &) {}
  virtual void visit(MatrixTransformNode &) {}
  virtual void visit(const MatrixTransformNode &) {}
  virtual void visit(TextureSamplingNode &) {}
  virtual void visit(const TextureSamplingNode &) {}
  virtual void visit(ScriptedNode &) {}
  virtual void visit(const ScriptedNode &) {}
  virtual void visit(CustomNode &) {}
  virtual void visit(const CustomNode &) {}

  virtual void visit(VertexMasterNode &) {}
  virtual void visit(const VertexMasterNode &) {}
  virtual void visit(SurfaceMasterNode &) {}
  virtual void visit(const SurfaceMasterNode &) {}
  virtual void visit(PostProcessMasterNode &) {}
  virtual void visit(const PostProcessMasterNode &) {}
};
