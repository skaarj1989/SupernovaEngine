#include "Nodes/VertexMaster.hpp"
#include "Utility.hpp"

const std::vector<VertexMasterNode::FieldInfo> VertexMasterNode::kFields{
  {"localPos", Attribute::Position},
};

//
// VertexMasterNode class:
//

VertexMasterNode::VertexMasterNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::None} {
  auto hint = vertex.id + 1;
  inputs.reserve(kFields.size());
  for (const auto &[name, value] : kFields) {
    auto *node = createNode(g, hint++, value);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> VertexMasterNode::clone(const IDPair) const {
  throw std::logic_error("Forbidden operation.");
}

std::string VertexMasterNode::toString() const { return "VertexMaster"; }
