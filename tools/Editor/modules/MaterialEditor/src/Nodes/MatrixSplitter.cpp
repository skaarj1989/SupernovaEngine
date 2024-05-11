#include "Nodes/MatrixSplitter.hpp"
#include "SplitMatrix.hpp"
#include "Nodes/Embedded.hpp"
#include "ShaderGraph.hpp"
#include <array>

namespace {

struct InputInfo {
  const char *name;
  SplitMatrix e;
};
constexpr std::array kFields = {
  InputInfo{.name = "[0]", .e = SplitMatrix::Column0},
  InputInfo{.name = "[1]", .e = SplitMatrix::Column1},
  InputInfo{.name = "[2]", .e = SplitMatrix::Column2},
  InputInfo{.name = "[3]", .e = SplitMatrix::Column3},
};

} // namespace

//
// MatrixSplitterNode class:
//

MatrixSplitterNode::MatrixSplitterNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Input} {
  auto hint = vertex.id + 1;
  outputs.reserve(kFields.size());
  for (const auto [name, e] : kFields) {
    auto *node = g.add<EmbeddedNode<SplitMatrix>>(hint++, e);
    node->label = name;
    node->flags |= NodeBase::Flags::Internal;
    outputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase> MatrixSplitterNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<MatrixSplitterNode>();
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string MatrixSplitterNode::toString() const { return "Split[matNxN]"; }
