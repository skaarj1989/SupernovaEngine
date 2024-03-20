#include "MaterialEditor/Nodes/Custom.hpp"
#include "VariantFromIndex.hpp"
#include "Utility.hpp"

CustomNode::CustomNode(ShaderGraph &g, const IDPair vertex,
                       const UserFunction::Handle userFunction_)
    : CompoundNode{g, vertex, Flags::Output}, userFunction{userFunction_} {
  auto hint = vertex.id + 1;
  inputs.reserve(userFunction.data->inputs.size());
  for (const auto &[name, dataType] : userFunction.data->inputs) {
    TransientVariant defaultValue{};
    if (!isSampler(dataType)) {
      defaultValue =
        variant_from_index<ValueVariant>(static_cast<int32_t>(dataType) - 1);
    }
    auto *node = createNode(g, hint++, defaultValue);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> CustomNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<CustomNode>();
  node->userFunction = userFunction;
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string CustomNode::toString() const {
  return userFunction.data ? userFunction.data->name : "(undefined)";
}
