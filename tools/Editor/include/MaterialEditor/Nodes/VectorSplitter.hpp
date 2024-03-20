#pragma once

#include "Compound.hpp"

class VectorSplitterNode : public CompoundNode {
public:
  VectorSplitterNode() = default;
  VectorSplitterNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
  }
};
