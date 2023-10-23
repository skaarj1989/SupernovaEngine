#include "MaterialEditor/Nodes/TextureSampling.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <format>

namespace {

[[nodiscard]] auto getRequiredUVType(DataType type) {
  using enum DataType;
  switch (type) {
  case Sampler2D:
    return Vec2;
  case SamplerCube:
    return Vec3;
  }
  return Undefined;
}

} // namespace

//
// TextureSamplingNode struct:
//

TextureSamplingNode TextureSamplingNode::create(ShaderGraph &g,
                                                VertexDescriptor parent) {
  const auto vd =
    g.addVertex(std::nullopt, NodeFlags::Internal | NodeFlags::Output);

  g.getVertexProp(vd).variant = VectorSplitterNode{
    .output =
      {
        .xyz = createInternalOutput(g, vd, std::nullopt, SplitVector::RGB),

        .x = createInternalOutput(g, vd, std::nullopt, SplitVector::R),
        .y = createInternalOutput(g, vd, std::nullopt, SplitVector::G),
        .z = createInternalOutput(g, vd, std::nullopt, SplitVector::B),
        .w = createInternalOutput(g, vd, std::nullopt, SplitVector::A),
      },
  };

  g.addEdge(EdgeType::Implicit, {.from = vd, .to = parent});

  return {
    .input =
      {
        .sampler = createInternalInput(g, parent, "sampler", std::monostate{}),
        .uv = createInternalInput(g, parent, "uv", Attribute::TexCoord0),
        .lod =
          {
            .vd = createInternalInput(g, parent, "lod", ValueVariant{0.0f}),
            .enabled = false,
          },
      },
    .split = vd,
  };
}
TextureSamplingNode TextureSamplingNode::clone(ShaderGraph &g,
                                               VertexDescriptor parent) const {
  auto node = create(g, parent);
  node.input.lod.enabled = input.lod.enabled;
  return node;
}

void TextureSamplingNode::remove(ShaderGraph &g) {
  removeNode(g, split);
  removeVertices(g, {input.sampler, input.uv, input.lod.vd});
}

bool TextureSamplingNode::inspect(ShaderGraph &graph, int32_t id) {
  ImNodes::BeginNodeTitleBar();
  constexpr auto kDefaultLabel = "TextureSampling";
  const auto titleBarWidth = ImGui::CalcTextSize(kDefaultLabel).x;
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // -- input:

  ImNodes::AddInputAttribute(graph.getVertexProp(input.sampler).id,
                             {.name = "sampler"});
  auto lastElementWidth = ImGui::GetItemRectSize().x;

  auto changed =
    addInputPin(graph, input.uv, {.name = "P"}, InspectorMode::Inline, false);
  lastElementWidth = glm::max(lastElementWidth, ImGui::GetItemRectSize().x);

  {
    auto &vertexProp = graph.getVertexProp(input.lod.vd);
    ImNodes::AddInputAttribute(vertexProp.id, {.name = "lod"}, [&](float) {
      constexpr auto kCheckboxLeftSide = false;
      constexpr auto kCheckboxRightSide = !kCheckboxLeftSide;

      const auto showCheckbox = [&] {
        ImGui::SameLine();
        changed |= ImGui::Checkbox(IM_UNIQUE_ID, &input.lod.enabled);
      };

      if constexpr (kCheckboxLeftSide) showCheckbox();

      if (const auto inspectLod = graph.countOutEdges(input.lod.vd) == 0;
          inspectLod) {
        ImGui::SameLine();

        auto &v = std::get<ValueVariant>(vertexProp.variant);
        ImGui::PushItemWidth(calcOptimalInspectorWidth(getDataType(v)));
        changed |= ::inspect(v);
        ImGui::PopItemWidth();
      }

      if constexpr (kCheckboxRightSide) showCheckbox();
    });
  }
  lastElementWidth = glm::max(lastElementWidth, ImGui::GetItemRectSize().x);

  constexpr auto kVerticalSpacing = 8.0f;
  ImGui::Dummy(ImVec2{0.0f, kVerticalSpacing});

  // -- output:

  const auto nodeWidth = glm::max(titleBarWidth, lastElementWidth);

  const auto &internalSplit = std::get<VectorSplitterNode>(
    std::get<CompoundNodeVariant>(graph.getVertexProp(split).variant));
  addOutputPins(graph, internalSplit.output, SwizzleMaskSet::RGBA, nodeWidth);

  ImGui::Dummy(ImVec2{0.0f, 5.0f});
  ImNodes::AddOutputAttribute(id, {.name = "out", .nodeWidth = nodeWidth});

  return changed;
}
NodeResult TextureSamplingNode::evaluate(MaterialGenerationContext &context,
                                         int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;

  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/texture.xhtml
  auto [samplerArg, uvArg, lodArg] = extract<3>(tokens);

  ShaderToken token{.name = nodeIdToString(id)};
  if (isSampler(samplerArg.dataType)) {
    if (const auto uvStr =
          assure(uvArg, getRequiredUVType(samplerArg.dataType));
        uvStr) {
      if (input.lod.enabled) {
        if (const auto lodStr = assure(lodArg, DataType::Float); lodStr) {
          token.dataType = DataType::Vec4;
          composer.addVariable(token.dataType, token.name,
                               std::format("textureLod({}, {}, {})",
                                           samplerArg.name, *uvStr, *lodStr));
        } else {
          return std::unexpected{"Invalid LOD."};
        }
      } else {
        token.dataType = DataType::Vec4;
        composer.addVariable(
          token.dataType, token.name,
          std::format("texture({}, {})", samplerArg.name, *uvStr));
      }
    } else {
      return std::unexpected{"Invalid UV."};
    }
  } else {
    return std::unexpected{"Not a sampler."};
  }

  return token;
}
