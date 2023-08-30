#include "MaterialEditor/Nodes/VectorSplitter.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

VectorSplitterNode VectorSplitterNode::create(ShaderGraph &g,
                                              VertexDescriptor parent) {
  return {
    .input = createInternalInput(g, parent, "in", std::monostate{}),
    .output =
      {
        .xyz = createInternalOutput(g, parent, "xyz", SplitVector::XYZ),

        .x = createInternalOutput(g, parent, "x", SplitVector::X),
        .y = createInternalOutput(g, parent, "y", SplitVector::Y),
        .z = createInternalOutput(g, parent, "z", SplitVector::Z),
        .w = createInternalOutput(g, parent, "w", SplitVector::W),
      },
  };
}
VectorSplitterNode VectorSplitterNode::clone(ShaderGraph &g,
                                             VertexDescriptor parent) const {
  return create(g, parent);
}

void VectorSplitterNode::remove(ShaderGraph &g) {
  if (input) g.removeVertex(*input);
  removeVertices(g, {
                      output.xyz,
                      output.x,
                      output.y,
                      output.z,
                      output.w,
                    });
}

bool VectorSplitterNode::inspect(ShaderGraph &g, [[maybe_unused]] int32_t id) {
  constexpr auto kDefaultLabel = "VectorSplitter";
  constexpr auto kMinNodeWidth = 50.0f;
  const auto nodeWidth =
    glm::max(ImGui::CalcTextSize(kDefaultLabel).x, kMinNodeWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // -- input:

  ImNodes::AddInputAttribute(g.getVertexProp(*input).id, {.name = "in"});

  constexpr auto kVerticalSpacing = 10.0f;
  ImGui::Dummy(ImVec2{nodeWidth, kVerticalSpacing});

  // -- output:

  addOutputPins(g, output, SwizzleMaskSet::XYZW, nodeWidth);

  return false;
}
NodeResult VectorSplitterNode::evaluate(MaterialGenerationContext &context,
                                        int32_t id) const {
  auto &tokens = context.currentShader->tokens;

  if (auto arg = extractTop(tokens); arg.isValid() && isVector(arg.dataType)) {
    return arg;
  } else {
    return std::unexpected{
      std::format("Cant split: {}.", toString(arg.dataType)),
    };
  }
}
