#pragma once

#include "Compound.hpp"

class ArithmeticNode : public CompoundNode {
public:
  enum class Operation { Add, Subtract, Multiply, Divide, COUNT };

  ArithmeticNode() = default;
  ArithmeticNode(ShaderGraph &, const IDPair, const Operation);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  Operation operation{Operation::Add};

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(operation);
  }
};

[[nodiscard]] const char *toString(const ArithmeticNode::Operation);
