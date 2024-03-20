#include "MaterialEditor/Nodes/PostProcessMaster.hpp"
#include "Utility.hpp"

const std::vector<PostProcessMasterNode::FieldInfo>
  PostProcessMasterNode::kFields{
    {"fragColor", glm::vec4{1.0f}},
  };

//
// PostProcessMasterNode class:
//

PostProcessMasterNode::PostProcessMasterNode(ShaderGraph &g,
                                             const IDPair vertex)
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

std::unique_ptr<NodeBase>
PostProcessMasterNode::clone(const IDPair cloned) const {
  throw std::logic_error("Forbidden operation.");
}

std::string PostProcessMasterNode::toString() const {
  return "PostProcessMaster";
}
