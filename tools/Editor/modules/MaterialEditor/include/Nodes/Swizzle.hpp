#pragma once

#include "Compound.hpp"

class SwizzleNode : public CompoundNode {
public:
  SwizzleNode() = default;
  SwizzleNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  static bool isMaskValid(const std::string &, const int32_t numChannels);

  // ---

  std::string mask;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(mask);
  }
};
