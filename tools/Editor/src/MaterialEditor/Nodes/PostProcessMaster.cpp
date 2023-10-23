#include "MaterialEditor/Nodes/PostProcessMaster.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include <format>

PostProcessMasterNode PostProcessMasterNode::create(ShaderGraph &g,
                                                    VertexDescriptor parent) {
  return {
    .fragColor = createInternalInput(g, parent, "fragColor",
                                     ValueVariant{glm::vec4{0.0f}}),
  };
}

bool PostProcessMasterNode::inspect(ShaderGraph &g,
                                    [[maybe_unused]] int32_t id) {
  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted("PostProcessMaster");
  ImNodes::EndNodeTitleBar();

  return addInputPin(g, fragColor, {.name = "FragColor"}, InspectorMode::Popup,
                     false);
}
MasterNodeResult
PostProcessMasterNode::evaluate(MaterialGenerationContext &context,
                                [[maybe_unused]] int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;

  const auto arg = extractTop(tokens);
  if (auto argStr = assure(arg, DataType::Vec4); argStr) {
    composer.addExpression(std::format("fragColor = {};", *argStr));
    return std::monostate{};
  } else {
    return std::unexpected{
      std::format("'fragColor': Can't convert {} -> vec4.",
                  toString(arg.dataType)),
    };
  }
}
