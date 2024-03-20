#pragma once

#include "Compound.hpp"

class TextureSamplingNode : public CompoundNode {
public:
  TextureSamplingNode() = default;
  TextureSamplingNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  bool useLOD{false};

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(useLOD);
  }
};
