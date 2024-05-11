#pragma once

#include "Compound.hpp"
#include "ScriptedFunction.hpp"

class ScriptedNode : public CompoundNode {
public:
  ScriptedNode() = default;
  ScriptedNode(ShaderGraph &, const IDPair, const ScriptedFunction::Handle);

  std::unique_ptr<NodeBase> clone(const IDPair) const override;

  void accept(NodeVisitor &visitor) override { visitor.visit(*this); }
  void accept(NodeVisitor &visitor) const override { visitor.visit(*this); }

  std::string toString() const override;

  // ---

  ScriptedFunction::Handle scriptedFunction;

  template <class Archive> void serialize(Archive &archive) {
    archive(cereal::base_class<CompoundNode>(this));
    archive(scriptedFunction.id);
  }
};
