#include "MaterialEditor/Nodes/MatrixTransform.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include "IconsFontAwesome6.h"
#include "MaterialEditor/ChangeEnumCombo.hpp"

namespace {

[[nodiscard]] constexpr auto makeHash(MatrixTransformNode::Space lhs,
                                      MatrixTransformNode::Space rhs) {
  return std::to_underlying(lhs) << 16 | std::to_underlying(rhs);
}

[[nodiscard]] std::pair<DataType, const char *>
findFunction(MatrixTransformNode::Space source,
             MatrixTransformNode::Space target) {
  using enum DataType;
  switch (makeHash(source, target)) {
    using enum MatrixTransformNode::Space;

  case makeHash(World, View):
    return {Vec3, "worldToView"};
  case makeHash(World, Clip):
    return {Vec4, "worldToClip"};
  case makeHash(World, NDC):
    return {Vec3, "worldToNDC"};

  case makeHash(View, World):
    return {Vec3, "viewToWorld"};
  case makeHash(View, Clip):
    return {Vec4, "viewToClip"};
  case makeHash(View, NDC):
    return {Vec3, "viewToNDC"};

    /*
  case makeHash(Clip, World):
    return {Vec4, "clipToWorld"};
  case makeHash(Clip, View):
    return {Vec4, "clipToView"};
    */

  case makeHash(NDC, View):
    return {Vec3, "NDCToView"};
  case makeHash(NDC, World):
    return {Vec3, "NDCToWorld"};
  }
  return {Undefined, "(not found)"};
}

[[nodiscard]] auto toString(MatrixTransformNode::Space space) {
  switch (space) {
    using enum MatrixTransformNode::Space;

  case Local:
    return "Local";
  case World:
    return "World";
  case View:
    return "View";
  case Clip:
    return "Clip";
  }
  assert(false);
  return "Undefined";
}

} // namespace

//
// MatrixTransformNode struct:
//

MatrixTransformNode MatrixTransformNode::create(ShaderGraph &g,
                                                VertexDescriptor parent) {
  return {
    .input = createInternalInput(g, parent, "input", std::monostate{}),
  };
}

MatrixTransformNode MatrixTransformNode::clone(ShaderGraph &g,
                                               VertexDescriptor parent) const {
  auto node = create(g, parent);
  node.source = source;
  node.target = target;
  copySimpleVariant(g, input, node.input);
  return node;
}

void MatrixTransformNode::remove(ShaderGraph &g) { g.removeVertex(input); }

bool MatrixTransformNode::inspect(ShaderGraph &g, int32_t id) {
  ImNodes::BeginNodeTitleBar();
  constexpr auto kDefaultLabel = "MatrixTransform";
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // ---

  ImNodes::AddInputAttribute(g.getVertexProp(input).id, {.name = "in"});

  ImGui::BeginGroup();
  constexpr auto kComboWidth = 45.0f;
  ImGui::PushItemWidth(kComboWidth);
  auto changed = changeEnumCombo(IM_UNIQUE_ID, source, toString,
                                 ImGuiComboFlags_NoArrowButton);

  ImGui::SameLine();
  ImGui::Text(ICON_FA_ARROW_RIGHT_LONG);

  ImGui::SameLine();
  changed = changeEnumCombo(IM_UNIQUE_ID, target, toString,
                            ImGuiComboFlags_NoArrowButton);
  ImGui::PopItemWidth();
  ImGui::EndGroup();
  const auto maxWidth = ImGui::GetItemRectSize().x;

  // ---

  ImNodes::AddOutputAttribute(id, {.name = "out", .nodeWidth = maxWidth});

  return changed;
}
NodeResult MatrixTransformNode::evaluate(MaterialGenerationContext &context,
                                         int32_t id) const {
  if (source == target) {
    return std::unexpected{"Do not use noop nodes."};
  }

  auto &[_, tokens, composer] = *context.currentShader;

  if (const auto arg = extractTop(tokens);
      arg.isValid() && arg.dataType == DataType::Vec4) {
    if (auto [returnType, fn] = findFunction(source, target);
        returnType != DataType::Undefined) {
      ShaderToken token{
        .name = nodeIdToString(id),
        .dataType = returnType,
      };
      composer.addVariable(token.dataType, token.name,
                           buildFunctionCall(fn, {arg.name}));
      return token;
    } else {
      return std::unexpected{"Could not find a function."};
    }
  }
  return std::unexpected{"Invalid input. Expected Vec4."};
}
