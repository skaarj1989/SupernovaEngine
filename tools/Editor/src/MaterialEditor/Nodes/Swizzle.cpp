#include "MaterialEditor/Nodes/Swizzle.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "imgui_stdlib.h" // InputTextWithHint
#include <format>

namespace {

[[nodiscard]] bool isMaskValid(const std::string &mask,
                               int32_t numInputChannels) {
  if (mask.empty()) return false;

  for (auto i = 1; const auto channel : {'x', 'y', 'z', 'w'}) {
    if (mask.contains(channel) && numInputChannels < i) {
      return false;
    }
    ++i;
  }
  return true;
}

} // namespace

//
// SwizzleNode struct:
//

SwizzleNode SwizzleNode::create(ShaderGraph &g, VertexDescriptor parent) {
  return {
    .input = createInternalInput(g, parent, std::nullopt, std::monostate{}),
  };
}
SwizzleNode SwizzleNode::clone(ShaderGraph &g, VertexDescriptor parent) const {
  auto node = SwizzleNode::create(g, parent);
  node.mask = mask;
  copySimpleVariant(g, input, node.input);
  return node;
}

void SwizzleNode::remove(ShaderGraph &g) { g.removeVertex(input); }

bool SwizzleNode::inspect(ShaderGraph &g, int32_t id) {
  ImNodes::BeginNodeTitleBar();
  constexpr auto kDefaultLabel = "Swizzle";
  ImGui::TextUnformatted(kDefaultLabel);
  ImNodes::EndNodeTitleBar();

  // -- input:

  ImNodes::AddInputAttribute(g.getVertexProp(input).id, {.name = "in"});

  // -- output:

  ImGui::SameLine();

  auto changed = false;

  ImNodes::AddOutputAttribute(id, true, [&] {
    constexpr auto kMaxSwizzleLength = 4;
    constexpr auto kWidthPerChar = 8.0f;
    ImGui::SetNextItemWidth(kWidthPerChar * kMaxSwizzleLength);
    changed = ImGui::InputTextWithHint(
      "out", "Mask", &mask, ImGuiInputTextFlags_CallbackCharFilter,
      +[](ImGuiInputTextCallbackData *data) {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
          switch (data->EventChar) {
          case 'x':
          case 'y':
          case 'z':
          case 'w':
            break;

          default:
            data->EventChar = 0;
          }
        }

        const auto ud = static_cast<const std::string *>(data->UserData);
        assert(ud);

        return ud->length() < kMaxSwizzleLength ? 0 : -1;
      },
      &mask);
  });

  return changed;
}
NodeResult SwizzleNode::evaluate(MaterialGenerationContext &context,
                                 int32_t id) const {
  auto &tokens = context.currentShader->tokens;

  if (const auto arg = extractTop(tokens);
      arg.isValid() && isMaskValid(mask, countChannels(arg.dataType))) {
    const auto baseType = getBaseDataType(arg.dataType);
    return ShaderToken{
      .name = std::format("{}.{}", arg.name, mask),
      .dataType = constructVectorType(baseType, mask.length()),
    };
  } else {
    return std::unexpected{
      std::format("Can't swizzle: {}.", toString(arg.dataType)),
    };
  }
}
