#include "Nodes/Append.hpp"
#include "ValueVariant.hpp"
#include "Nodes/Embedded.hpp"
#include "ShaderGraph.hpp"

AppendNode::AppendNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Output} {
  auto hint = vertex.id + 1;
  inputs.reserve(2);
  for (const auto name : {"A", "B"}) {
    auto *node = g.add<EmbeddedNode<ValueVariant>>(hint++, 0.0f);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> AppendNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<AppendNode>();
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string AppendNode::toString() const { return "Append"; }
