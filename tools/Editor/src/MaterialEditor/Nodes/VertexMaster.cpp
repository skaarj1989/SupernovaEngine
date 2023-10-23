#include "MaterialEditor/Nodes/VertexMaster.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <format>

namespace {

constexpr std::pair kField{"localPos", DataType::Vec4};

} // namespace

VertexMasterNode VertexMasterNode::create(ShaderGraph &g,
                                          VertexDescriptor parent) {
  return VertexMasterNode{
    .localPos =
      createInternalInput(g, parent, kField.first, Attribute::Position),
  };
}

bool VertexMasterNode::inspect(ShaderGraph &g, [[maybe_unused]] int32_t id) {
  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted("VertexMaster");
  ImNodes::EndNodeTitleBar();

  auto changed = false;

#define ADD_INPUT_PIN(name)                                                    \
  changed |= addInputPin(g, name, {#name}, InspectorMode::Popup, false)

  ADD_INPUT_PIN(localPos);

#undef ADD_INPUT_PIN

  return changed;
}
MasterNodeResult VertexMasterNode::evaluate(MaterialGenerationContext &context,
                                            int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;
  const auto [name, requiredType] = kField;

  const auto arg = extractTop(tokens);
  if (auto argStr = assure(arg, requiredType); argStr) {
    composer.addExpression(std::format("{} = {};", name, *argStr));
  } else {
    return std::unexpected{
      std::format("'{}': Can't convert {} -> {}.", name, toString(arg.dataType),
                  toString(requiredType)),
    };
  }

  return std::monostate{};
}
