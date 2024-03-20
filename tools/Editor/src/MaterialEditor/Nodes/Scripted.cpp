#include "MaterialEditor/Nodes/Scripted.hpp"
#include "Utility.hpp"

ScriptedNode::ScriptedNode(ShaderGraph &g, const IDPair vertex,
                           const ScriptedFunction::Handle scriptedFunction_)
    : CompoundNode{g, vertex, Flags::Output},
      scriptedFunction{scriptedFunction_} {
  auto hint = vertex.id + 1;
  inputs.reserve(scriptedFunction.data->args.size());
  for (const auto &arg : scriptedFunction.data->args) {
    auto *node = createNode(g, hint++, arg.defaultValue);
    node->label = arg.name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> ScriptedNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<ScriptedNode>();
  node->scriptedFunction = scriptedFunction;
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string ScriptedNode::toString() const {
  return scriptedFunction.data ? scriptedFunction.data->name : "(undefined)";
}
