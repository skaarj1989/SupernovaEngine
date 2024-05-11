#pragma once

#include "NodeBase.hpp"

class CompoundNode : public NodeBase {
public:
  CompoundNode() = default;
  CompoundNode(ShaderGraph &, const IDPair, const Flags);

  void joinChildren() override;
  void collectChildren(std::vector<VertexDescriptor> &) const override;

  // ---

  std::vector<IDPair> inputs;
  std::vector<IDPair> outputs;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<NodeBase>(this));
    archive(inputs, outputs);
  }
};

void cloneChildren(const CompoundNode &, CompoundNode &,
                   std::optional<VertexID> hint);
