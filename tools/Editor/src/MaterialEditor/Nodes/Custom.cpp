
#include "MaterialEditor/Nodes/Custom.hpp"
#include "VariantFromIndex.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "ImGuiHelper.hpp" // ShowTooltip

namespace {

[[nodiscard]] auto sameTypes(std::span<const DataType> a,
                             std::span<const UserFunctionData::Parameter> b) {
  assert(a.size() == b.size());
  return std::ranges::equal(
    a, b, [](auto lhs, const auto &rhs) { return lhs == rhs.dataType; });
}

} // namespace

//
// CustomNode struct:
//

CustomNode
CustomNode::create(ShaderGraph &g, VertexDescriptor parent,
                   std::pair<std::size_t, const UserFunctionData *> p) {
  CustomNode node{.hash = p.first, .data = p.second};
  node.inputs.reserve(node.data->inputs.size());
  std::ranges::transform(
    node.data->inputs, std::back_inserter(node.inputs), [&](const auto &arg) {
      auto defaultValue =
        !isSampler(arg.dataType)
          ? VertexProp::Variant{variant_from_index<ValueVariant>(
              static_cast<int32_t>(arg.dataType) - 1)}
          : std::monostate{};
      return createInternalInput(g, parent, arg.name, defaultValue);
    });
  return node;
}
CustomNode CustomNode::clone(ShaderGraph &g, VertexDescriptor parent) const {
  return create(g, parent, {*hash, data});
}

bool CustomNode::inspect(ShaderGraph &g, int32_t id) {
  assert(data);

  constexpr auto kMinTitleBarWidth = 80.0f;
  const auto cstr = data->name.c_str();
  const auto titleBarWidth =
    glm::max(ImGui::CalcTextSize(cstr).x, kMinTitleBarWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(cstr);
  ImNodes::EndNodeTitleBar();

  if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered())
    ImGui::ShowTooltip(data->code.c_str());

  auto maxWidth = 0.0f;

  // -- input:

  auto changed = false;

  for (auto [i, vd] : std::views::enumerate(inputs)) {
    const auto &arg = data->inputs[i];
    changed |=
      addInputPin(g, vd, {.name = arg.name}, InspectorMode::Inline, false);
    maxWidth = glm::max(maxWidth, ImGui::GetItemRectSize().x);
  }

  ImGui::Spacing();

  // -- output:

  const auto nodeWidth = glm::max(titleBarWidth, maxWidth);
  ImNodes::AddOutputAttribute(id, {
                                    .name = toString(data->output),
                                    .nodeWidth = nodeWidth,
                                  });

  return changed;
}
NodeResult CustomNode::evaluate(MaterialGenerationContext &context,
                                int32_t id) const {
  const auto numArguments = inputs.size();
  assert(numArguments == data->inputs.size());

  auto &[_, tokens, composer] = *context.currentShader;

  std::vector<ShaderToken> args;
  if (numArguments > 0) args = extractN(tokens, numArguments);

  if (auto const argTypes = extractArgTypes(args);
      sameTypes(argTypes, data->inputs)) {
    composer.addFunction(data);

    ShaderToken token{
      .name = nodeIdToString(id),
      .dataType = data->output,
    };
    composer.addVariable(token.dataType, token.name,
                         buildFunctionCall(data->name, extractArgNames(args)));
    return token;
  } else {
    return std::unexpected{
      std::format("'{}' No matching overloaded function found.",
                  buildFunctionCall(data->name, transform(argTypes))),
    };
  }
}

bool CustomNode::setFunction(const UserFunctions &functions) {
  if (const auto it = functions.find(*hash); it != functions.cend()) {
    data = it->second.get();
    return true;
  }
  return false;
}
