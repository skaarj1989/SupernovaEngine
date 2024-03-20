#include "MaterialEditor/Nodes/NodeBase.hpp"
#include "MaterialEditor/ShaderGraph.hpp"

NodeBase::NodeBase(ShaderGraph &g, const IDPair vertex_, const Flags flags_)
    : graph{&g}, vertex{vertex_}, flags{flags_} {
  using enum Flags;
  assert((flags & (Input | Output)) != (Input | Output));
}

rhi::ShaderType NodeBase::getOrigin() const { return graph->getShaderType(); }

std::optional<std::string> NodeBase::setLabel(std::string label_) {
  if (label != label_) {
    std::swap(label, label_);
    publish(NodeLabelUpdatedEvent{});
    markDirty();
    return label_;
  }
  return std::nullopt;
}
void NodeBase::markDirty() { publish(NodeValueUpdatedEvent{}); }

bool NodeBase::isLinkedToRoot() const { return graph->isLinkedToRoot(vertex); }
bool NodeBase::hasConnectedInput() const {
  return graph->countOutEdges(vertex) > 0;
}
