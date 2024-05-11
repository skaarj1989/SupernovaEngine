#pragma once

#include "Compound.hpp"
#include "TransientVariant.hpp"

class VertexMasterNode : public CompoundNode {
public:
  VertexMasterNode() = default;
  VertexMasterNode(ShaderGraph &, const IDPair);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  struct FieldInfo {
    const char *name;
    TransientVariant defaultValue;
  };
  static const std::vector<FieldInfo> kFields;

  // ---

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
  }
};
