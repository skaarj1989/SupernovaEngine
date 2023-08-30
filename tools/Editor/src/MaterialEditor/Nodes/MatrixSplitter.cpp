#include "MaterialEditor/Nodes/MatrixSplitter.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

namespace {

void addOutputPins(ShaderGraph &g, const MatrixSplitterNode::Output &out,
                   float nodeWidth) {
  using Item =
    std::tuple<VertexDescriptor, std::string_view, std::optional<ImU32>>;
  std::initializer_list<Item> pins{
    {out.column0, "[0]", ImColor{1.0f, 0.2f, 0.321f}},
    {out.column1, "[1]", ImColor{0.545f, 0.862f, 0.0f}},
    {out.column2, "[2]", ImColor{0.156f, 0.563f, 1.0f}},
    {out.column3, "[3]", IM_COL32(255, 255, 255, 127)},
  };

  for (const auto &[vd, label, color] : pins) {
    const auto &vertexProp = g.getVertexProp(vd);
    ImNodes::AddOutputAttribute(vertexProp.id, {
                                                 .name = label,
                                                 .color = color,
                                                 .nodeWidth = nodeWidth,
                                               });
  }
}

} // namespace

//
// MatrixSplitterNode struct:
//

MatrixSplitterNode MatrixSplitterNode::create(ShaderGraph &g,
                                              VertexDescriptor parent) {
  using enum SplitMatrix;
  return {
    .input = createInternalInput(g, parent, "in", std::monostate{}),
    .output =
      {
        .column0 = createInternalOutput(g, parent, "[0]", Column0),
        .column1 = createInternalOutput(g, parent, "[1]", Column1),
        .column2 = createInternalOutput(g, parent, "[2]", Column2),
        .column3 = createInternalOutput(g, parent, "[3]", Column3),
      },
  };
}
MatrixSplitterNode MatrixSplitterNode::clone(ShaderGraph &g,
                                             VertexDescriptor parent) const {
  return create(g, parent);
}

void MatrixSplitterNode::remove(ShaderGraph &g) {
  removeVertices(g, {
                      input,
                      output.column0,
                      output.column1,
                      output.column2,
                      output.column3,
                    });
}

bool MatrixSplitterNode::inspect(ShaderGraph &g, int32_t) {
  constexpr auto kDefaultLabel = "MatrixSplitter";
  constexpr auto kMinNodeWidth = 50.0f;
  const auto nodeWidth =
    glm::max(ImGui::CalcTextSize(kDefaultLabel).x, kMinNodeWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // -- input:

  ImNodes::AddInputAttribute(g.getVertexProp(input).id, {.name = "in"});

  constexpr auto kVerticalSpacing = 10.0f;
  ImGui::Dummy(ImVec2{nodeWidth, kVerticalSpacing});

  // -- output:

  addOutputPins(g, output, nodeWidth);

  return false;
}
NodeResult MatrixSplitterNode::evaluate(MaterialGenerationContext &context,
                                        int32_t id) const {
  auto &tokens = context.currentShader->tokens;

  if (auto arg = extractTop(tokens); arg.isValid() && isMatrix(arg.dataType)) {
    return arg;
  } else {
    return std::unexpected{
      std::format("Cant split: {}.", toString(arg.dataType)),
    };
  }
}
