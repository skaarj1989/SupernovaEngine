#pragma once

#include "NodeBase.hpp"

class EmptyNode : public NodeBase {
public:
  EmptyNode() = default;
  EmptyNode(ShaderGraph &g, const IDPair vertex)
      : NodeBase{g, vertex, Flags::None} {}

  std::unique_ptr<NodeBase> clone(const IDPair cloned) const override {
    return std::make_unique<EmptyNode>(*graph, cloned);
  }

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override { return "EmptyNode"; }

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<NodeBase>(this));
  }
};
