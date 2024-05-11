#include "Nodes/Compound.hpp"
#include "ShaderGraph.hpp"
#include <algorithm>
#include <ranges>

CompoundNode::CompoundNode(ShaderGraph &g, const IDPair vertex,
                           const Flags flags_)
    : NodeBase{g, vertex, flags_} {}

void CompoundNode::joinChildren() {
  for (auto &child : inputs) {
    graph->updateVertexDescriptor(child);
    graph->link(EdgeType::Internal, {vertex.vd, child.vd});
  }
  for (auto &child : outputs) {
    graph->updateVertexDescriptor(child);
    graph->link(EdgeType::Internal, {child.vd, vertex.vd});
  }
}
void CompoundNode::collectChildren(std::vector<VertexDescriptor> &out) const {
  for (const auto &set : {inputs, outputs}) {
    for (const auto [vd, _] : set)
      graph->collectChildren(vd, out);
  }
}

//
// Helper:
//

void cloneChildren(const CompoundNode &src, CompoundNode &dest,
                   std::optional<VertexID> hint) {
  const auto cloneChild = [&g = *src.graph, &hint](const IDPair &in) {
    const auto out = g.clone(in.vd, hint);
    if (hint) ++(*hint);
    return out;
  };
  dest.inputs.reserve(src.inputs.size());
  std::ranges::transform(src.inputs, std::back_inserter(dest.inputs),
                         cloneChild);

  dest.outputs.reserve(src.outputs.size());
  std::ranges::transform(src.outputs, std::back_inserter(dest.outputs),
                         cloneChild);
}
