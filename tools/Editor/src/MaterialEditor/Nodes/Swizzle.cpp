#include "MaterialEditor/Nodes/Swizzle.hpp"
#include "MaterialEditor/Nodes/Empty.hpp"
#include "MaterialEditor/ShaderGraph.hpp"

SwizzleNode::SwizzleNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Output} {
  auto *node = g.add<EmptyNode>(vertex.id + 1);
  node->label = "in";
  node->flags = Flags::Internal | Flags::Input;
  inputs.emplace_back(node->vertex);
}

std::unique_ptr<NodeBase> SwizzleNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<SwizzleNode>();
  node->mask = mask;
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string SwizzleNode::toString() const { return "Swizzle[vecN]"; }

bool SwizzleNode::isMaskValid(const std::string &mask,
                              const int32_t numChannels) {
  if (mask.empty() || mask.length() > 4) return false;

  for (auto i = 1; const auto channel : {'x', 'y', 'z', 'w'}) {
    if (mask.contains(channel) && numChannels < i) {
      return false;
    }
    ++i;
  }
  return true;
}
