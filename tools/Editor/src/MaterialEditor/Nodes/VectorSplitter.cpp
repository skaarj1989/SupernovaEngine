#include "MaterialEditor/Nodes/VectorSplitter.hpp"
#include "MaterialEditor/SplitVector.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"
#include "MaterialEditor/ShaderGraph.hpp"
#include <array>

namespace {

struct InputInfo {
  const char *name;
  SplitVector e;
};
constexpr std::array kFields = {
  InputInfo{.name = "xyz", .e = SplitVector::XYZ},
  InputInfo{.name = "x", .e = SplitVector::X},
  InputInfo{.name = "y", .e = SplitVector::Y},
  InputInfo{.name = "z", .e = SplitVector::Z},
  InputInfo{.name = "w", .e = SplitVector::W},
};

} // namespace

//
// VectorSplitterNode class:
//

VectorSplitterNode::VectorSplitterNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Input} {
  auto hint = vertex.id + 1;
  outputs.reserve(kFields.size());
  for (const auto [name, e] : kFields) {
    auto *node = g.add<EmbeddedNode<SplitVector>>(hint++, e);
    node->label = name;
    node->flags |= NodeBase::Flags::Internal;
    outputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> VectorSplitterNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<VectorSplitterNode>();
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string VectorSplitterNode::toString() const { return "Split[vecN]"; }
