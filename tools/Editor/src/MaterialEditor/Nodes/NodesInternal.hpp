#include "MaterialEditor/ShaderGraph.hpp"
#include "MaterialEditor/ImNodesHelper.hpp"

[[nodiscard]] std::string
buildFunctionCall(const std::string_view functionName,
                  const std::vector<std::string> &args);

enum class GraphFlow { Input = 0, Output = 1 };

template <typename T>
[[nodiscard]] auto createInternalVertex(ShaderGraph &g, VertexDescriptor parent,
                                        std::optional<std::string_view> label,
                                        GraphFlow uiFlow, T value) {
  assert(parent);

  const auto vd = g.addVertex(
    label, NodeFlags::Internal | NodeFlags{1 << std::to_underlying(uiFlow)});

  if constexpr (std::is_convertible_v<T, VertexProp::Variant>) {
    g.getVertexProp(vd).variant = std::move(value);
  }
  g.addEdge(EdgeType::Implicit, uiFlow == GraphFlow::Input
                                  ? Connection{.from = parent, .to = vd}
                                  : Connection{.from = vd, .to = parent});

  return vd;
}

template <typename T>
[[nodiscard]] auto createInternalInput(ShaderGraph &g, VertexDescriptor parent,
                                       std::optional<std::string_view> label,
                                       T &&value) {
  return createInternalVertex(g, parent, label, GraphFlow::Input,
                              std::forward<T>(value));
}
template <typename T>
[[nodiscard]] auto createInternalOutput(ShaderGraph &g, VertexDescriptor parent,
                                        std::optional<std::string_view> label,
                                        T &&value) {
  return createInternalVertex(g, parent, label, GraphFlow::Output,
                              std::forward<T>(value));
}

enum class InspectorMode { Inline, Popup };
[[nodiscard]] bool addInputPin(ShaderGraph &, VertexDescriptor,
                               const ImNodes::InputAttributeParams &,
                               InspectorMode, bool allowTypeChange = true);

enum class SwizzleMaskSet { XYZW, RGBA, STPQ };
void addOutputPins(ShaderGraph &, const VectorSplitterNode::Output &,
                   SwizzleMaskSet, float nodeWidth);

void copySimpleVariant(ShaderGraph &, VertexDescriptor sourceVd,
                       VertexDescriptor targetVd);
