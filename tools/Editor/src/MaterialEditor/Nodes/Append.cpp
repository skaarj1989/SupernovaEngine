#include "MaterialEditor/Nodes/Append.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include "ImGuiHelper.hpp"

#include <format>

namespace {

[[nodiscard]] std::optional<DataType> combine(DataType a, DataType b) {
  const auto baseType = isVector(a) ? getBaseDataType(a) : a;
  const auto outVectorSize = countChannels(a) + countChannels(b);
  return outVectorSize > 0 && outVectorSize <= 4
           ? std::optional{constructVectorType(baseType, outVectorSize)}
           : std::nullopt;
}

[[nodiscard]] std::optional<DataType> combine(const ShaderToken &a,
                                              const ShaderToken &b) {
  const auto qualified = [](const ShaderToken &token) {
    return token.isValid() &&
           (isScalar(token.dataType) || isVector(token.dataType));
  };
  return qualified(a) && qualified(b) ? combine(a.dataType, b.dataType)
                                      : std::nullopt;
}

} // namespace

//
// AppendNode struct:
//

AppendNode AppendNode::create(ShaderGraph &g, VertexDescriptor parent) {
  return {
    .input =
      {
        .a = createInternalInput(g, parent, "a", ValueVariant{0.0f}),
        .b = createInternalInput(g, parent, "b", ValueVariant{0.0f}),
      },
  };
}
AppendNode AppendNode::clone(ShaderGraph &g, VertexDescriptor parent) const {
  auto node = create(g, parent);
  copySimpleVariant(g, input.a, node.input.a);
  copySimpleVariant(g, input.b, node.input.b);
  return node;
}

void AppendNode::remove(ShaderGraph &g) {
  removeVertices(g, {input.a, input.b});
}

bool AppendNode::inspect(ShaderGraph &g, int32_t id) {
  constexpr auto kDefaultLabel = "Append";
  const auto titleBarWidth = ImGui::CalcTextSize(kDefaultLabel).x;

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // -- input:

  auto changed = addInputPin(g, input.a, {.name = "A"}, InspectorMode::Inline);
  auto maxWidth = ImGui::GetItemRectSize().x;

  changed |= addInputPin(g, input.b, {.name = "B"}, InspectorMode::Inline);
  maxWidth = glm::max(maxWidth, ImGui::GetItemRectSize().x);

  ImGui::Spacing();

  // -- output:

  ImNodes::AddOutputAttribute(
    id, {.name = "out", .nodeWidth = glm::max(titleBarWidth, maxWidth)});

  return changed;
}
NodeResult AppendNode::evaluate(MaterialGenerationContext &context,
                                int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;
  auto [aArg, bArg] = extract<2>(tokens);

  if (const auto dataType = combine(aArg, bArg); dataType) {
    ShaderToken token{
      .name = nodeIdToString(id),
      .dataType = *dataType,
    };
    composer.addVariable(token.dataType, token.name,
                         std::format("{}({}, {})", toString(token.dataType),
                                     aArg.name, bArg.name));
    return token;
  } else {
    return std::unexpected{
      std::format("Can't append: {} to {}.", toString(bArg.dataType),
                  toString(aArg.dataType)),
    };
  }
}
