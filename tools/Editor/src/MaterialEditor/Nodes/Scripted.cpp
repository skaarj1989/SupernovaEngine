#include "MaterialEditor/Nodes/Scripted.hpp"
#include "VariantCast.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "ImGuiHelper.hpp" // ShowTooltip

ScriptedNode
ScriptedNode::create(ShaderGraph &g, VertexDescriptor parent,
                     std::pair<uint32_t, const ScriptedFunctionData *> p) {
  ScriptedNode node{.guid = p.first, .data = p.second};
  assert(node.data);

  node.inputs.resize(node.data->args.size());
  std::ranges::transform(
    node.data->args, node.inputs.begin(), [&](const auto &arg) {
      return createInternalInput(g, parent, arg.name.c_str(),
                                 variant_cast(arg.defaultValue));
    });

  return node;
}
ScriptedNode ScriptedNode::clone(ShaderGraph &g,
                                 VertexDescriptor parent) const {
  if (!data) return {};

  auto node = create(g, parent, {*guid, data});
  for (auto [i, src] : std::views::enumerate(inputs)) {
    copySimpleVariant(g, src, node.inputs[i]);
  }
  return node;
}

bool ScriptedNode::inspect(ShaderGraph &graph, int32_t id) {
  const auto cstr = toString();
  constexpr auto kMinNodeWidth = 80.0f;
  const auto nodeWidth = glm::max(ImGui::CalcTextSize(cstr).x, kMinNodeWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(cstr);
  ImNodes::EndNodeTitleBar();

  if (data && ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered())
    ImGui::ShowTooltip(data->description);

  auto maxWidth = 0.0f;

  // -- input:

  auto changed = false;

  for (auto [i, vd] : std::views::enumerate(inputs)) {
    const auto &arg =
      data ? data->args[i]
           : ScriptedFunctionData::Parameter{.name = std::format("arg{}", i)};

    changed |=
      addInputPin(graph, vd, {.name = arg.name}, InspectorMode::Inline);
    maxWidth = glm::max(maxWidth, ImGui::GetItemRectSize().x);

    if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered())
      ImGui::ShowTooltip(arg.description);

    ++i;
  }

  ImGui::Spacing();

  // -- output:

  ImNodes::AddOutputAttribute(
    id, {.name = "out", .nodeWidth = glm::max(nodeWidth, maxWidth)});

  return changed;
}
NodeResult ScriptedNode::evaluate(MaterialGenerationContext &context,
                                  int32_t id) const {
  if (!data) return std::unexpected("Invalid data.");

  auto &[_, tokens, composer] = *context.currentShader;

  std::vector<ShaderToken> args;
  if (!inputs.empty()) args = extractN(tokens, inputs.size());
  const auto argTypes = extractArgTypes(args);

  if (const auto returnType = data->getReturnTypeCb(argTypes);
      returnType != DataType::Undefined) {
    ShaderToken token{
      .name = nodeIdToString(id),
      .dataType = returnType,
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

const char *ScriptedNode::toString() const {
  return data ? data->name.c_str() : "Invalid";
}

bool ScriptedNode::setFunction(const ScriptedFunctions &functions) {
  if (const auto it = functions.find(*guid); it != functions.cend()) {
    data = it->second.get();
    return true;
  }
  return false;
}
