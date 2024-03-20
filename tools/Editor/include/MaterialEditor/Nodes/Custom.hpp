#pragma once

#include "Compound.hpp"
#include "MaterialEditor/UserFunction.hpp"

class CustomNode : public CompoundNode {
public:
  CustomNode() = default;
  CustomNode(ShaderGraph &, const IDPair, const UserFunction::Handle);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  UserFunction::Handle userFunction;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(userFunction.id);
  }
};
