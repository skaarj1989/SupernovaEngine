#include "Nodes/MatrixTransform.hpp"
#include "Nodes/Empty.hpp"
#include "ShaderGraph.hpp"

MatrixTransformNode::MatrixTransformNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Output} {
  auto *node = g.add<EmptyNode>(vertex.id + 1);
  node->flags = Flags::Internal | Flags::Input;
  inputs.emplace_back(node->vertex);
}

std::unique_ptr<NodeBase>
MatrixTransformNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<MatrixTransformNode>();
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string MatrixTransformNode::toString() const {
  return "Transform[matNxN]";
}
