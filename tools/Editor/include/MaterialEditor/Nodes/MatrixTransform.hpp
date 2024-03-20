#pragma once

#include "Compound.hpp"

class MatrixTransformNode : public CompoundNode {
public:
  MatrixTransformNode() = default;
  MatrixTransformNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  enum class Space { Local, World, View, Clip, NDC, COUNT };
  Space source{Space::Local};
  Space target{Space::World};

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(source, target);
  }
};
