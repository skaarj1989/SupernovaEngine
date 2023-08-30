#include "MaterialEditor/Nodes/Container.hpp"
#include "TypeTraits.hpp" // is_any_v
#include "NodesInternal.hpp"

namespace {

[[nodiscard]] auto getLine(const VertexProp::Variant &variant) {
  return std::visit(
    [](const auto &arg) -> std::pair<DataType, std::string> {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (is_any_v<T, FrameBlockMember, CameraBlockMember>) {
        return {getDataType(arg), toString(arg)};
      }
      return {DataType::Undefined, "Unknown"};
    },
    variant);
}

struct OutputAttribute {
  int32_t id{-1};
  DataType dataType{DataType::Undefined};
  std::string label;
  float lineWidth{0.0f};
};
[[nodiscard]] auto buildOutputAttributes(ShaderGraph &g,
                                         std::span<VertexDescriptor> items) {
  std::vector<OutputAttribute> v(items.size());
  std::ranges::transform(items, v.begin(), [&](const auto vd) {
    const auto &vertexProp = g.getVertexProp(vd);

    auto [dataType, label] = getLine(vertexProp.variant);
    const auto lineWidth = ImGui::CalcTextSize(toString(dataType)).x +
                           ImGui::CalcTextSize(label.c_str()).x;

    return OutputAttribute{
      .id = vertexProp.id,
      .dataType = dataType,
      .label = std::move(label),
      .lineWidth = lineWidth,
    };
  });

  return v;
}

[[nodiscard]] auto findMaxWidth(std::span<const OutputAttribute> items) {
  auto maxWidth = 0.0f;
  for (const auto &item : items) {
    maxWidth = glm::max(item.lineWidth, maxWidth);
  }
  return maxWidth;
}

void printMember(DataType dataType, const std::string_view name) {
  static const ImColor kTypeKeywordColor{86, 156, 214};
  ImGui::TextColored(kTypeKeywordColor, toString(dataType));
  ImGui::SameLine();
  ImGui::TextUnformatted(name.data());
}

} // namespace

//
// ContainerNode struct:
//

ContainerNode ContainerNode::clone(ShaderGraph &g,
                                   VertexDescriptor parent) const {
  ContainerNode node{.name = name};
  node.outputs.reserve(outputs.size());
  for (auto [i, src] : std::views::enumerate(outputs)) {
    auto vd = createInternalOutput(g, parent, std::nullopt, std::nullopt);
    copySimpleVariant(g, src, node.outputs.emplace_back(vd));
  }
  return node;
}

void ContainerNode::remove(ShaderGraph &g) { removeVertices(g, outputs); }

bool ContainerNode::inspect(ShaderGraph &g, [[maybe_unused]] int32_t) {
  const auto spaceWidth = ImGui::CalcTextSize(" ").x;
  const auto outputAttributes = buildOutputAttributes(g, outputs);
  const auto maxWidth = findMaxWidth(outputAttributes) + spaceWidth;

  const auto nodeWidth =
    glm::max(ImGui::CalcTextSize(name.c_str()).x, maxWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(name.c_str());
  ImNodes::EndNodeTitleBar();

  // -- output:

  for (const auto &[id, dataType, label, lineWidth] : outputAttributes) {
    ImNodes::BeginOutputAttribute(id);

    const auto w = glm::max(nodeWidth - lineWidth, 0.0f);
    ImGui::Indent(w);
    printMember(dataType, label);
    ImGui::Unindent(w);

    ImNodes::EndOutputAttribute();
  }

  return false;
}
