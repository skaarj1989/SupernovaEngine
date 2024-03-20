#pragma once

#include "Compound.hpp"

class AppendNode : public CompoundNode {
public:
  AppendNode() = default;
  AppendNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
  }
};
