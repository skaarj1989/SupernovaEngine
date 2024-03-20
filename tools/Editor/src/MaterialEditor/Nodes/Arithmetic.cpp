#include "MaterialEditor/Nodes/Arithmetic.hpp"
#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"
#include "MaterialEditor/ShaderGraph.hpp"

ArithmeticNode::ArithmeticNode(ShaderGraph &g, const IDPair vertex,
                               const Operation operation_)
    : CompoundNode{g, vertex, Flags::Output}, operation{operation_} {
  auto hint = vertex.id + 1;
  inputs.reserve(2);
  for (const auto name : {"A", "B"}) {
    auto *node = g.add<EmbeddedNode<ValueVariant>>(hint++, 0.0f);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> ArithmeticNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<ArithmeticNode>();
  node->operation = operation;
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string ArithmeticNode::toString() const { return ::toString(operation); }

//
// Helper:
//

const char *toString(const ArithmeticNode::Operation op) {
#define CASE(Value)                                                            \
  case ArithmeticNode::Operation::Value:                                       \
    return #Value

  switch (op) {
    CASE(Add);
    CASE(Subtract);
    CASE(Multiply);
    CASE(Divide);
  }
#undef CASE

  assert(false);
  return "Undefined";
}
